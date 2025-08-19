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

//����һ�����������������ͨ��
typedef struct NetClient {
    QActive super;
    struct tcp_pcb *tcp_conn; // lwIP TCP���ӿ��ƿ�
    
    elab_device_t *run_led;
    
    char uuid[64];
    
    uint8_t frame_head[8];   // ���ڹ���websocket֡ͷ
    uint32_t frame_head_len; // ��Чwebsocket֡ͷ����
    uint8_t  masking_key[4]; // websocket֡����
    
    uint8_t *frame_payload;   // ��Ҫ���͵ľ�������
    uint32_t payload_len;     // �������ݳ��� 
    
	QTimeEvt timer;           // ��ʱ����������
    uint8_t missedHeartbeats; // δ�յ���Ӧ����������
} NetClient;

//��ʼ��
static QState NetClient_initial(NetClient * const me, void const * const pre);
static QState NetClient_work(NetClient * const me, QEvt const * const e);
static QState NetClient_connecting(NetClient * const me, QEvt const * const e);
static QState NetClient_connected(NetClient * const me, QEvt const * const e);

//����ص�����
static void onError(void *arg, err_t err);
//���ӻص�����
static err_t onConnected(void *arg, struct tcp_pcb *tpcb, err_t err);
//���ջص�����
static err_t onReceive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
//��ʱ����ص�����
static err_t onPoll(void *arg, struct tcp_pcb *tpcb);
//�¼����ͺ���
static void sendData(NetClient * const me, const uint8_t *data, uint16_t len);
//���ݴ�����
static void processData(NetClient * const me, uint8_t *data, uint16_t len);
//�������JSON��Ϣ
static void processJson(NetClient * const me, char *json_str);

//�������ͺ���
static void sendHeartbeat(NetClient * const me);
static void sendButton(NetClient * const me, RsltRecon_t *recon, int type);
static void sendAssembly(NetClient * const me, int8_t torque);

//����֡ͷ
static int createFrameHead(NetClient * const me, uint32_t msg_len);
//����֡����
static void createFramePayLoad(NetClient * const me, char *message);
//��������֡������
static void createFrameAndSend(NetClient * const me, char *json_str);

static NetClient l_netclinet;//����NetClient����
QActive *const AO_NetClient = &l_netclinet.super;//����NetClient�����ָ��

// ��ʼ������ͨ�Ż����
void NetClient_ctor(void) {
	NetClient *me = &l_netclinet;
    
    QActive_ctor(&me->super, Q_STATE_CAST(&NetClient_initial));
    QTimeEvt_ctorX(&me->timer, AO_NetClient, TIMEOUT_SIG, 0);  //������ʱ��
    
//    init_cjson_hooks();
}

//ʵ��״̬��������ͬ�������¼�: ��ʼ��
static QState NetClient_initial(NetClient * const me, void const * const pre) {
    (void)pre;
    
    me->tcp_conn = NULL;
	me->missedHeartbeats = 0;
    memset(me->uuid, 0, sizeof(me->uuid));
    
    me->run_led = elab_device_find("led_run");
    elab_assert(me->run_led != NULL);
	
    //��Ϊ���ֳɹ�������������ʱ
    //QTimeEvt_armX(&me->timer, TIMEOUT_INTERVAL, TIMEOUT_INTERVAL);
    
    return Q_TRAN(&NetClient_work);
}

//ʵ��״̬��������ͬ�������¼�: ����״̬
static QState NetClient_work(NetClient * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&QHsm_top);
    
    switch (e->sig)
    {
        case Q_INIT_SIG:
            //elog_debug("NetClient_work: Q_TRAN NetClient_connecting!\n");
            //elab_led_toggle(me->run_led, 4000);
            ret = Q_TRAN(&NetClient_connecting);//��ʼת������������״̬
         break;
    }
    
    return ret;
}

//ʵ��״̬��������ͬ�������¼�: ��������
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
		
            //ע���쳣����
            tcp_err(me->tcp_conn, onError); 

            elab_led_toggle(me->run_led, 2000);
            
            ret = Q_HANDLED();
            break;
        }
        case CONNECT_SIG:{
            // ���ӳɹ�������������״̬
            //elog_debug("NetClient_connecting: now connected!\n");
            ret = Q_TRAN(&NetClient_connected);
            break;
        }
        
        case DISCONNECT_SIG:{
            // ���Ӳ��ɹ����رղ�����
            //elog_debug("NetClient_connecting: now disconnected!\n");
            //�رձ�������
            tcp_close(me->tcp_conn);
            me->tcp_conn = NULL;
            ret = Q_TRAN(&NetClient_work);
            break;
        }
    }
    
    return ret;
}

//ʵ��״̬��������ͬ�������¼�: ������
static QState NetClient_connected(NetClient * const me, QEvt const * const e) {
    
    QState ret = Q_SUPER(&NetClient_work);
    
    switch (e->sig) 
    {
		case Q_ENTRY_SIG:
            //elog_debug("NetClient_connected: Q_ENTRY_SIG!\n");
            // �����ѽ�������������������
            me->missedHeartbeats = 0;
        
            elab_led_toggle(me->run_led, 500);
            ret = Q_HANDLED();
            break;
        
        case TIMEOUT_SIG:
            // ������������
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
        
        case DISCONNECT_SIG:{ //�ر�����
            // �Ͽ�����
            //elog_debug("NetClient_connected: DISCONNECT_SIG!\n");
            QTimeEvt_disarm(&me->timer);//�ر�������ʱ��
            tcp_close(me->tcp_conn);
            me->tcp_conn = NULL;
            ret = Q_TRAN(&NetClient_work);
            break;
        }
      
        case DATA_RECEIVED_SIG:{
            //elog_debug("NetClient_connected: DATA_RECEIVED_SIG!\n");
            DataRecvEvt *dataEvt = (DataRecvEvt *)e;
            //������յ�������
            processData(me, (uint8_t *)dataEvt->data, dataEvt->len);
            ret = Q_HANDLED();
           
			break;
        }
        
		case RESULT_OPERATION_SIG: {
            //elog_debug("NetClient_connected: RESULT_OPERATION_SIG!\n");
            RsltOperationEvt *RsltOpEvt = (RsltOperationEvt *)e;
            //TODO:����ϱ���������
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

//����lwIP�Ļص��������������
static void onError(void *arg, err_t err)
{
    elog_debug("connect error! closed by core, err is %d!\n", err);
	//�����ӶϿ���Ϣ
    QACTIVE_POST(AO_NetClient, Q_NEW(QEvt, DISCONNECT_SIG), 0);    
}

//����lwIP�Ļص���������������
static err_t onConnected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    
    if (err == ERR_OK) 
    {
        elog_debug("onConnected: Connected to WebSocket server\n");

        // ����WebSocket��������
        const char *handshake_request =
            "GET /ws HTTP/1.1\r\n"
            "Host: "SERVER_IP":7272\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n" // ʾ����Կ
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n";

        u16_t len = strlen(handshake_request);
        tcp_write(tpcb, handshake_request, len, TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        
        //ע�ᶨʱ������
        //tcp_poll(tpcb, onPoll, 6);
	
        //ע��һ�����պ���
        tcp_recv(tpcb, onReceive);
	
        //�������ӳɹ���Ϣ
        QACTIVE_POST(AO_NetClient, Q_NEW(QEvt, CONNECT_SIG), 0);
    }
    else 
    {
        elog_debug("onConnected: Failed to connect: %d\n", err);
        QACTIVE_POST(AO_NetClient, Q_NEW(QEvt, DISCONNECT_SIG), 0);   
    }
   
    return ERR_OK;
}

//��ʱ����ص�����
static err_t onPoll(void *arg, struct tcp_pcb *tpcb)
{
  //uint8_t send_buf[]= "This is a TCP Client test...\n";
  
  //�������ݵ�������
  //tcp_write(tpcb, send_buf, sizeof(send_buf), 1); 
  
  //���������һЩ��Ҫ��ʱ�ظ������ݣ�����WebSocket��POLL
    
  return ERR_OK;
}

// lwIP��TCP���ջص��������������
static err_t onReceive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) 
{   
    //elog_debug("client_recv!\n");
    //NetClient *me = (NetClient*)arg;
    
    if (p != NULL) 
	{        
		//��������
		tcp_recved(tpcb, p->tot_len);
        
        //TODO �������ʵ�ʽ����վݴ���	
        if (p->len < MAX_DATARECV_SIZE) 
        {
            DataRecvEvt *dataEvt = Q_NEW(DataRecvEvt, DATA_RECEIVED_SIG);
            memcpy(dataEvt->data, p->payload, p->len); // ��������
            dataEvt->len = p->len;
            //����DATA_RECEIVED_SIG�¼�
            QACTIVE_POST(AO_NetClient, (QEvt *)dataEvt, 0);
        }
		
		memset(p->payload, 0 , p->len);
		pbuf_free(p);
	} 
	else if (err != ERR_OK) 
	{
		//�������Ͽ�����
		elog_debug("server has been disconnected!\n");	
		//�����ӶϿ���Ϣ
		QACTIVE_POST(AO_NetClient, Q_NEW(QEvt, DISCONNECT_SIG), 0);   
	}
    
    return ERR_OK;
}


//���Ͳ�װ�������
static void sendAssembly(NetClient * const me, int8_t torque)
{   
    char *json_str = create_assembly_result(me->uuid, torque);
    elab_assert(json_str != NULL);
    elog_debug("sendAssembly: json_str is %d: %s\n", strlen(json_str), json_str);
    
    createFrameAndSend(me, json_str);
    vPortFree(json_str);
}

//����
static void bntSort(Button_t arr[], int n) {
    int i, j;
    Button_t temp = {0};
    for (i = 0; i < n-1; i++) {     
        // ���i��Ԫ���Ѿ����ź�����ˣ�����Ҫ�ٱȽ�
        for (j = 0; j < n-i-1; j++) {
            if (arr[j].seqs > arr[j+1].seqs) {
                // ����Ԫ�������Ƚ�
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

// �ȽϺ���������qsort
static int compare(const void *a, const void *b) {
    const Button_t *buttonA = (const Button_t *)a;
    const Button_t *buttonB = (const Button_t *)b;
    return buttonA->seqs - buttonB->seqs;
}

//����ʶ��������

static void sendButton(NetClient * const me, RsltRecon_t *recon, int type)
{
    Button_t bnts[NUM_BUTTONS] = {0};
    
    int idx = 0;
    
    for (uint8_t i = 1; i <= NUM_BUTTONS && idx < recon->num_bnts; i++) {
        if (recon->seq_bnts[i] != 0) {
            bnts[idx].index = i; // ��ť���
            bnts[idx].seqs = recon->seq_bnts[i]; //����˳��
            bnts[idx].type = recon->op_bnts[i];    //��������
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

//ʵ�ַ��������ʹ������ݵĺ���
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

//��������֡������
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

//����Websocket֡ͷ
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

//����֡����
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

//���ݷ��ͺ���
static void sendData(NetClient * const me, const uint8_t *data, uint16_t len) {
	//��֯HTTPЭ������
    
    if (me->tcp_conn != NULL) {
        tcp_write(me->tcp_conn, data, len, TCP_WRITE_FLAG_COPY); 
        tcp_output(me->tcp_conn);
    }
}

//���ݴ�����
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
     //��Чjson����  
    if (start == 0 || end == 0)
        return; 
    
    //����websocketЭ��ĵ�2���ֽڣ����ݳ��������롮{���ַ�ֵ��ͬ
    if (start == 1)
        start = 2; 
    
    data[end + 1] = '\0';
    int type = parse_command_type((char*)&data[start]);
    
    //��Ч����
    if (-1 == type)
        return;
    
    if (type == CMD_TYPE_CONNECT){
        //�������ֳɹ�
        elog_debug("processData:CMD_TYPE_CONNECT:%s!\n", (char*)&data[start]);	
        QTimeEvt_armX(&me->timer, TIMEOUT_INTERVAL, TIMEOUT_INTERVAL);
        
        //TODO: ˢ��ϵͳʱ��
        SynTimeEvt *SynTmEvt = Q_NEW(SynTimeEvt, SYN_TIME_SIG);
        SynTmEvt->time = parse_command_syntime((char*)&data[start]);
        QACTIVE_POST(AO_Operater, (QEvt *)SynTmEvt, 0);
    }    
    else if (type == CMD_TYPE_HEARTBEAT){
        // ������Ӧ����������������
        //elog_debug("processData:CMD_TYPE_HEARTBEAT!\n");	
        me->missedHeartbeats = 0;
    }
    else if (type == CMD_TYPE_END){ //��������
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
             //TODO:������Ӧ����������̣�����Ϣ
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
