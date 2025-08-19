#include "app.h"
#include "data_parse.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "common/elab_assert.h"
#include "elab/common/elab_log.h"
#include "elab/edf/elab_device.h"
#include "elab/edf/user/elab_led.h"
#include "FreeRTOS.h"
#include <string.h>
#include <time.h>

ELAB_TAG("NetClient");
Q_DEFINE_THIS_MODULE("NetClient")

//定义一个活动对象来管理网络通信
typedef struct NetClient {
    QActive super;
    struct tcp_pcb *tcp_conn; // lwIP TCP连接控制块
    
    elab_device_t *run_led;
    
    char uuid[64];
    
    uint8_t frame_head[8];   // 用于构建websocket帧头
    uint32_t frame_head_len; // 有效websocket帧头长度
    uint8_t  masking_key[4]; // websocket帧掩码
    
    uint8_t *frame_payload;   // 将要发送的净荷数据
    uint32_t payload_len;     // 净荷数据长度 
    
	QTimeEvt timer;           // 定时器：周期性
    uint8_t missedHeartbeats; // 未收到响应的心跳次数
} NetClient;

//初始化
static QState NetClient_initial(NetClient * const me, void const * const pre);
static QState NetClient_work(NetClient * const me, QEvt const * const e);
static QState NetClient_connecting(NetClient * const me, QEvt const * const e);
static QState NetClient_connected(NetClient * const me, QEvt const * const e);

//错误回调函数
static void onError(void *arg, err_t err);
//连接回调函数
static err_t onConnected(void *arg, struct tcp_pcb *tpcb, err_t err);
//接收回调函数
static err_t onReceive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
//定时处理回调函数
static err_t onPoll(void *arg, struct tcp_pcb *tpcb);
//事件发送函数
static void sendData(NetClient * const me, const uint8_t *data, uint16_t len);
//数据处理函数
static void processData(NetClient * const me, uint8_t *data, uint16_t len);
//处理解析JSON消息
static void processJson(NetClient * const me, char *json_str);

//心跳发送函数
static void sendHeartbeat(NetClient * const me);
static void sendButton(NetClient * const me, RsltRecon_t *recon, int type);
static void sendAssembly(NetClient * const me, int8_t torque);

//创建帧头
static int createFrameHead(NetClient * const me, uint32_t msg_len);
//创建帧净荷
static void createFramePayLoad(NetClient * const me, char *message);
//创建数据帧并发送
static void createFrameAndSend(NetClient * const me, char *json_str);

static NetClient l_netclinet;//定义NetClient对象
QActive *const AO_NetClient = &l_netclinet.super;//定义NetClient活动对象指针

// 初始化网络通信活动对象
void NetClient_ctor(void) {
	NetClient *me = &l_netclinet;
    
    QActive_ctor(&me->super, Q_STATE_CAST(&NetClient_initial));
    QTimeEvt_ctorX(&me->timer, AO_NetClient, TIMEOUT_SIG, 0);  //创建定时器
    
//    init_cjson_hooks();
}

//实现状态机来处理不同的网络事件: 初始化
static QState NetClient_initial(NetClient * const me, void const * const pre) {
    (void)pre;
    
    me->tcp_conn = NULL;
	me->missedHeartbeats = 0;
    memset(me->uuid, 0, sizeof(me->uuid));
    
    me->run_led = elab_device_find("led_run");
    elab_assert(me->run_led != NULL);
	
    //改为握手成功才启动心跳定时
    //QTimeEvt_armX(&me->timer, TIMEOUT_INTERVAL, TIMEOUT_INTERVAL);
    
    return Q_TRAN(&NetClient_work);
}

//实现状态机来处理不同的网络事件: 工作状态
static QState NetClient_work(NetClient * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&QHsm_top);
    
    switch (e->sig)
    {
        case Q_INIT_SIG:
            //elog_debug("NetClient_work: Q_TRAN NetClient_connecting!\n");
            //elab_led_toggle(me->run_led, 4000);
            ret = Q_TRAN(&NetClient_connecting);//初始转换到建立连接状态
         break;
    }
    
    return ret;
}

//实现状态机来处理不同的网络事件: 建立连接
static QState NetClient_connecting(NetClient * const me, QEvt const * const e) {
    
    QState ret = Q_SUPER(&NetClient_work);
    
    switch (e->sig) 
    {   
        case Q_ENTRY_SIG:{
            //elog_debug("NetClient_connecting: Q_ENTRY_SIG!\n");
            me->tcp_conn = tcp_new();
            elab_assert(me->tcp_conn != NULL);
            //tcp_arg(me->tcp_conn, me);        
		
            ip4_addr_t server_ip;
            ipaddr_aton(SERVER_IP, &server_ip);
            
            elog_debug("NetClient_connecting: now tcp_connect!\n");
            tcp_connect(me->tcp_conn, &server_ip, SERVER_PORT, onConnected);
		
            //注册异常处理
            tcp_err(me->tcp_conn, onError); 

            elab_led_toggle(me->run_led, 2000);
            
            ret = Q_HANDLED();
            break;
        }
        case CONNECT_SIG:{
            // 连接成功，进入已连接状态
            //elog_debug("NetClient_connecting: now connected!\n");
            ret = Q_TRAN(&NetClient_connected);
            break;
        }
        
        case DISCONNECT_SIG:{
            // 连接不成功，关闭并重连
            //elog_debug("NetClient_connecting: now disconnected!\n");
            //关闭本地连接
            tcp_close(me->tcp_conn);
            me->tcp_conn = NULL;
            ret = Q_TRAN(&NetClient_work);
            break;
        }
    }
    
    return ret;
}

//实现状态机来处理不同的网络事件: 连接中
static QState NetClient_connected(NetClient * const me, QEvt const * const e) {
    
    QState ret = Q_SUPER(&NetClient_work);
    
    switch (e->sig) 
    {
		case Q_ENTRY_SIG:
            //elog_debug("NetClient_connected: Q_ENTRY_SIG!\n");
            // 连接已建立，重置心跳计数器
            me->missedHeartbeats = 0;
        
            elab_led_toggle(me->run_led, 500);
            ret = Q_HANDLED();
            break;
        
        case TIMEOUT_SIG:
            // 发送心跳请求
            //elog_debug("NetClient_connected: TIMEOUT_SIG!\n");
            if (me->missedHeartbeats < HEARTBEAR_REPEAT_TIMES)
            {
               sendHeartbeat(me);
               me->missedHeartbeats++;
               ret = Q_HANDLED();
            }
            else
            {
                //elog_debug("NetClient_connected: Heartbeat Timeout!\n");
                //tcp_close(me->tcp_conn);
                //ret = Q_TRAN(&NetClient_work);
                QACTIVE_POST(AO_NetClient, Q_NEW(QEvt, DISCONNECT_SIG), 0); 
            }
            
            break;
        
        case DISCONNECT_SIG:{ //关闭连接
            // 断开连接
            //elog_debug("NetClient_connected: DISCONNECT_SIG!\n");
            QTimeEvt_disarm(&me->timer);//关闭心跳定时器
            tcp_close(me->tcp_conn);
            me->tcp_conn = NULL;
            ret = Q_TRAN(&NetClient_work);
            break;
        }
      
        case DATA_RECEIVED_SIG:{
            //elog_debug("NetClient_connected: DATA_RECEIVED_SIG!\n");
            DataRecvEvt *dataEvt = (DataRecvEvt *)e;
            //处理接收到的数据
            processData(me, (uint8_t *)dataEvt->data, dataEvt->len);
            ret = Q_HANDLED();
           
			break;
        }
        
		case RESULT_OPERATION_SIG: {
            //elog_debug("NetClient_connected: RESULT_OPERATION_SIG!\n");
            RsltOperationEvt *RsltOpEvt = (RsltOperationEvt *)e;
            //TODO:组包上报给服务器
            if (RsltOpEvt->type == CMD_TYPE_RECOGNITION || RsltOpEvt->type == CMD_TYPE_RESET)
            {
                sendButton(me, &RsltOpEvt->value.recon, RsltOpEvt->type);
            }
            else if (RsltOpEvt->type == CMD_TYPE_ASSEMBLY)
            {
                sendAssembly(me, RsltOpEvt->value.torque);
            }
            
            memset(me->uuid, 0, sizeof(me->uuid));
            ret = Q_HANDLED();
            break;
        }
    }
    
    return ret;
}

//定义lwIP的回调函数来处理错误
static void onError(void *arg, err_t err)
{
    elog_debug("connect error! closed by core, err is %d!\n", err);
	//发连接断开消息
    QACTIVE_POST(AO_NetClient, Q_NEW(QEvt, DISCONNECT_SIG), 0);    
}

//定义lwIP的回调函数来处理连接
static err_t onConnected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    
    if (err == ERR_OK) 
    {
        elog_debug("onConnected: Connected to WebSocket server\n");

        // 发送WebSocket握手请求
        const char *handshake_request =
            "GET /ws HTTP/1.1\r\n"
            "Host: "SERVER_IP":7272\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n" // 示例密钥
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n";

        u16_t len = strlen(handshake_request);
        tcp_write(tpcb, handshake_request, len, TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        
        //注册定时处理函数
        //tcp_poll(tpcb, onPoll, 6);
	
        //注册一个接收函数
        tcp_recv(tpcb, onReceive);
	
        //发出连接成功消息
        QACTIVE_POST(AO_NetClient, Q_NEW(QEvt, CONNECT_SIG), 0);
    }
    else 
    {
        elog_debug("onConnected: Failed to connect: %d\n", err);
        QACTIVE_POST(AO_NetClient, Q_NEW(QEvt, DISCONNECT_SIG), 0);   
    }
   
    return ERR_OK;
}

//定时处理回调函数
static err_t onPoll(void *arg, struct tcp_pcb *tpcb)
{
  //uint8_t send_buf[]= "This is a TCP Client test...\n";
  
  //发送数据到服务器
  //tcp_write(tpcb, send_buf, sizeof(send_buf), 1); 
  
  //这里可以做一些需要定时回复的数据，例如WebSocket的POLL
    
  return ERR_OK;
}

// lwIP的TCP接收回调函数来处理接收
static err_t onReceive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) 
{   
    //elog_debug("client_recv!\n");
    //NetClient *me = (NetClient*)arg;
    
    if (p != NULL) 
	{        
		//接收数据
		tcp_recved(tpcb, p->tot_len);
        
        //TODO 这里进行实际接收收据处理	
        if (p->len < MAX_DATARECV_SIZE) 
        {
            DataRecvEvt *dataEvt = Q_NEW(DataRecvEvt, DATA_RECEIVED_SIG);
            memcpy(dataEvt->data, p->payload, p->len); // 复制数据
            dataEvt->len = p->len;
            //发布DATA_RECEIVED_SIG事件
            QACTIVE_POST(AO_NetClient, (QEvt *)dataEvt, 0);
        }
		
		memset(p->payload, 0 , p->len);
		pbuf_free(p);
	} 
	else if (err != ERR_OK) 
	{
		//服务器断开连接
		elog_debug("server has been disconnected!\n");	
		//发连接断开消息
		QACTIVE_POST(AO_NetClient, Q_NEW(QEvt, DISCONNECT_SIG), 0);   
	}
    
    return ERR_OK;
}


//发送拆装操作结果
static void sendAssembly(NetClient * const me, int8_t torque)
{   
    char *json_str = create_assembly_result(me->uuid, torque);
    elab_assert(json_str != NULL);
    elog_debug("sendAssembly: json_str is %d: %s\n", strlen(json_str), json_str);
    
    createFrameAndSend(me, json_str);
    vPortFree(json_str);
}

//排序
static void bntSort(Button_t arr[], int n) {
    int i, j;
    Button_t temp = {0};
    for (i = 0; i < n-1; i++) {     
        // 最后i个元素已经是排好序的了，不需要再比较
        for (j = 0; j < n-i-1; j++) {
            if (arr[j].seqs > arr[j+1].seqs) {
                // 相邻元素两两比较
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

// 比较函数，用于qsort
static int compare(const void *a, const void *b) {
    const Button_t *buttonA = (const Button_t *)a;
    const Button_t *buttonB = (const Button_t *)b;
    return buttonA->seqs - buttonB->seqs;
}

//发送识别操作结果

static void sendButton(NetClient * const me, RsltRecon_t *recon, int type)
{
    Button_t bnts[NUM_BUTTONS] = {0};
    
    int idx = 0;
    
    for (uint8_t i = 1; i <= NUM_BUTTONS && idx < recon->num_bnts; i++) {
        if (recon->seq_bnts[i] != 0) {
            bnts[idx].index = i; // 按钮编号
            bnts[idx].seqs = recon->seq_bnts[i]; //按下顺序
            bnts[idx].type = recon->op_bnts[i];    //操作类型
            idx++;
        }
    }
    /*
    elog_debug("idx is %d\n", idx);
    for (uint8_t i = 0; i < idx; i++)
    {
        elog_debug("index is %d, seqs is %d\n",  bnts[i].index, bnts[i].seqs);
    }
    */
    //qsort(bnts, idx, sizeof(Button_t), compare);
    bntSort(bnts, idx); 
    
    char *json_str = create_button_result(me->uuid, bnts, idx, type);
    elab_assert(json_str != NULL);
    elog_debug("sendButton: json_str is %d: %s\n", strlen(json_str), json_str);
    
    createFrameAndSend(me, json_str);
    vPortFree(json_str);
}

//实现发送心跳和处理数据的函数
static void sendHeartbeat(NetClient * const me) 
{
   
    char *json_str = create_heartbeat_request();
    elab_assert(json_str != NULL);
    //Selog_debug("sendHeartbeat: json_str is %d: %s\n", strlen(json_str),  json_str);
    
    //char http_request[128];
    //int len = create_http_request(http_request, sizeof(http_request), HEARTBEAT_URL, json_str); 
 
    createFrameAndSend(me, json_str);
    vPortFree(json_str);
 }

//创建数据帧并发送
static void createFrameAndSend(NetClient * const me, char *json_str)
{
    if (0 == createFrameHead(me, strlen(json_str)))
    {
        createFramePayLoad(me, json_str);
    }
    
    if (me->tcp_conn != NULL && me->frame_head_len != 0)
    {
        tcp_write(me->tcp_conn, me->frame_head, me->frame_head_len, TCP_WRITE_FLAG_COPY);
        tcp_write(me->tcp_conn, me->frame_payload, me->payload_len, TCP_WRITE_FLAG_COPY);
        tcp_output(me->tcp_conn);
    }
}

//创建Websocket帧头
static int createFrameHead(NetClient * const me, uint32_t msg_len)
{
    elab_assert(me != NULL);
    
    if (msg_len <= 125)
    {
        me->frame_head_len = 6;
    }
    else if (msg_len <= 65535)
    {
        me->frame_head_len = 8;
    }
    else
    {
        me->frame_head_len = 0;
        elog_debug("createFrameHead: msg is so big!\n");
        return -1;
    }
    
    // Generate a random masking key
    srand((unsigned int)(xTaskGetTickCount() / 1000));
    for (int i = 0; i < 4; i++) {
        me->masking_key[i] = (uint8_t)(rand() % 256);
    }
    
    if (msg_len <= 125)
    {        
        me->frame_head[0] = 0x81;// FIN bit set, opcode 1 (text)
        me->frame_head[1] = 0x80 | (uint8_t)msg_len; // MASK bit set
        // Add masking key to the frame
        me->frame_head[2] = me->masking_key[0];
        me->frame_head[3]  = me->masking_key[1];
        me->frame_head[4] = me->masking_key[2];
        me->frame_head[5] = me->masking_key[3];
    }
    else if (msg_len <= 65535) 
    {
        me->frame_head[0] = 0x81;// FIN bit set, opcode 1 (text)
        me->frame_head[1] = 0x80 | 126; // MASK bit set
        me->frame_head[2] = (uint8_t)(( msg_len >> 8) & 0xFF);
        me->frame_head[3] = (uint8_t)(msg_len & 0xFF);
        // Add masking key to the frame
        me->frame_head[4] = me->masking_key[0];
        me->frame_head[5]  = me->masking_key[1];
        me->frame_head[6] = me->masking_key[2];
        me->frame_head[7] = me->masking_key[3];
    }
    
    return 0;
}

//创建帧净荷
static void createFramePayLoad(NetClient * const me, char *message) 
{
    uint32_t mes_len = strlen(message);

    // Apply masking key to the payload data
    for (size_t i = 0; i < mes_len; i++) {
        message[i] ^= me->masking_key[i % 4];
    }
    
    me->frame_payload = (uint8_t*)message;
    me->payload_len = mes_len;
}

//数据发送函数
static void sendData(NetClient * const me, const uint8_t *data, uint16_t len) {
	//组织HTTP协议数据
    
    if (me->tcp_conn != NULL) {
        tcp_write(me->tcp_conn, data, len, TCP_WRITE_FLAG_COPY); 
        tcp_output(me->tcp_conn);
    }
}

//数据处理函数
static void processData(NetClient * const me, uint8_t *data, uint16_t len) {
    (void)me;

    int start = 0, end = 0;
      
    for(int i = 0; i < len; i++)
    {
        if (start == 0 && data[i] == '{')
                start = i;
        
        if (end == 0 && data[len - 1 - i] == '}')
            end = len - 1 - i;
        
        if (start != 0 && end != 0)
            break;
    }
     //有效json数据  
    if (start == 0 || end == 0)
        return; 
    
    //避免websocket协议的第2个字节，数据长度正好与‘{’字符值相同
    if (start == 1)
        start = 2; 
    
    data[end + 1] = '\0';
    int type = parse_command_type((char*)&data[start]);
    
    //有效类型
    if (-1 == type)
        return;
    
    if (type == CMD_TYPE_CONNECT){
        //连接握手成功
        elog_debug("processData:CMD_TYPE_CONNECT:%s!\n", (char*)&data[start]);	
        QTimeEvt_armX(&me->timer, TIMEOUT_INTERVAL, TIMEOUT_INTERVAL);
        
        //TODO: 刷新系统时间
        SynTimeEvt *SynTmEvt = Q_NEW(SynTimeEvt, SYN_TIME_SIG);
        SynTmEvt->time = parse_command_syntime((char*)&data[start]);
        QACTIVE_POST(AO_Operater, (QEvt *)SynTmEvt, 0);
    }    
    else if (type == CMD_TYPE_HEARTBEAT){
        // 心跳响应，重置心跳计数器
        //elog_debug("processData:CMD_TYPE_HEARTBEAT!\n");	
        me->missedHeartbeats = 0;
    }
    else if (type == CMD_TYPE_END){ //结束操作
        elog_debug("processData:CMD_TYPE_END:%s!\n", (char*)&data[start]);	
        QF_publish_(Q_NEW(QEvt, END_OPERATION_SIG));
    }
    else{
          int time = parse_command_duration((char*)&data[start]);
          parse_command_uuid((char*)&data[start], me->uuid);

          if (time > 0)
          {
              elog_debug("processData: CMD_TYPE_COMMAND:%s\n", (char*)&data[start]);
             //elog_debug("processData: time is %d\n", time);
             //TODO:启动相应命令操作流程：发消息
             CmdOperationEvt *CmdOpEvt = Q_NEW(CmdOperationEvt, COMMAND_OPERATION_SIG);
             CmdOpEvt->type = type;
             CmdOpEvt->duration= time;
     
             QACTIVE_POST(AO_Operater, (QEvt *)CmdOpEvt, 0);
          }
          else
          {
              elog_debug("processData:parse_command_duration failed!\n");	
          }
    }     
}
