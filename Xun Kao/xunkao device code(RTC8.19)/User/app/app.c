#include "app.h"
#include "elab/common/elab_export.h"
#include "elab/3rd/qpc/include/qpc.h"
#include "sys_arch.h"
#include "elab/os/cmsis_os.h"
#include "elab/edf/user/elab_led.h"
#include "data_parse.h"
#include "cJSON.h"

Q_DEFINE_THIS_MODULE("APP")

static QSubscrList subscrSto[MAX_PUB_SIG];

//事件队列
static QEvt const *l_netClientSto[3]; // 根据需要调整大小
static QEvt const *l_operaterSto[5]; // 根据需要调整大小

//事件事件池
static QF_MPOOL_EL(RsltOperationEvt)    l_smlPoolSto[5];
static QF_MPOOL_EL(DataRecvEvt)         l_medPoolSto[5];

static TaskHandle_t NetTaskCreate_Handle = NULL;

//应用初始化
static void app_init(void)
{ 
	static uint8_t netClientStk[3 * 1024];
    static uint8_t operaterStk[512];
    
    init_cjson_hooks();
    
    Operater_ctor();
    NetClient_ctor();
	
    QF_poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));
    QF_poolInit(l_medPoolSto, sizeof(l_medPoolSto), sizeof(l_medPoolSto[0]));
    
    QF_psInit(subscrSto, Q_DIM(subscrSto));
                    
    QACTIVE_START(AO_Operater,
                    osPriorityNormal,                            /* QP priority */
                    l_operaterSto, Q_DIM(l_operaterSto),        /* evt queue */
                    (void *)operaterStk, sizeof(operaterStk),                   /* thread stack */
                    (QEvt *)0);                                 /* no initialization event */
                    
    QACTIVE_START(AO_NetClient,
                    osPriorityAboveNormal,                        /* QP priority */
                    l_netClientSto, Q_DIM(l_netClientSto),        /* evt queue */
                    (void *)netClientStk, sizeof(netClientStk),                   /* thread stack */
                    (QEvt *)0);                                   /* no initialization event */
}

static void NetTaskCreate(void)
{    
    //elab_led_toggle(elab_device_find("led_G"), 500);
    //elab_led_toggle(elab_device_find("led_run"), 2000);
    
    TCPIP_Init();
    app_init();

    vTaskDelete(NetTaskCreate_Handle); //删除AppTaskCreate任务
}

//网络初始化
static void net_init(void)
{   
    xTaskCreate((TaskFunction_t )NetTaskCreate,  /* 任务入口函数 */
                        (const char*    )"NetTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&NetTaskCreate_Handle);/* 任务控制块指针 */ 
}
INIT_EXPORT(net_init, EXPORT_APP);


