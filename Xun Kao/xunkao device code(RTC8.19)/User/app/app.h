#ifndef APP_H
#define APP_H

#include "qpc.h"
#include "data_parse.h"

#include "../driver/drv_pin.h"

#define SERVER_IP          "192.168.1.131"//+++++++++++++++++++++++++++++++++++++++++++++++++++++++101.37.119.120"
#define SERVER_PORT        7272            //5001
#define TIMEOUT_INTERVAL   5000
#define HEARTBEAR_REPEAT_TIMES 3

#define MAX_DATARECV_SIZE 156

#define NUM_BUTTONS   MAX_KEY_IN //识别按钮个数

 /* signals used in the app */
enum AppSignals{                  
	TIME_TICK_SIG = Q_USER_SIG,
    
    BUTTON_KEY1_SIG,   //按钮1
    BUTTON_KEY2_SIG,
    BUTTON_KEY3_SIG,
    BUTTON_KEY4_SIG,
    BUTTON_KEY5_SIG,
    BUTTON_KEY6_SIG,
    BUTTON_KEY7_SIG,
    BUTTON_KEY8_SIG,
    BUTTON_KEY9_SIG,
    BUTTON_KEY10_SIG,
    BUTTON_KEY11_SIG,
    BUTTON_KEY12_SIG,
    BUTTON_KEY13_SIG,
    BUTTON_KEY14_SIG,
    BUTTON_KEY15_SIG,
    BUTTON_KEY16_SIG,
    BUTTON_KEY17_SIG,
    BUTTON_KEY18_SIG,
    BUTTON_KEY19_SIG,
    BUTTON_KEY20_SIG,
	BUTTON_KEY21_SIG,
    BUTTON_KEY22_SIG,
    BUTTON_KEY23_SIG,
    BUTTON_KEY24_SIG,
    
    RELEASE_KEY1_SIG,   //按钮1
    RELEASE_KEY2_SIG,
    RELEASE_KEY3_SIG,
    RELEASE_KEY4_SIG,
    RELEASE_KEY5_SIG,
    RELEASE_KEY6_SIG,
    RELEASE_KEY7_SIG,
    RELEASE_KEY8_SIG,
    RELEASE_KEY9_SIG,
    RELEASE_KEY10_SIG,
    RELEASE_KEY11_SIG,
    RELEASE_KEY12_SIG,
    RELEASE_KEY13_SIG,
    RELEASE_KEY14_SIG,
    RELEASE_KEY15_SIG,
    RELEASE_KEY16_SIG,
    RELEASE_KEY17_SIG,
    RELEASE_KEY18_SIG,
    RELEASE_KEY19_SIG,
    RELEASE_KEY20_SIG,
	RELEASE_KEY21_SIG,
    RELEASE_KEY22_SIG,
    RELEASE_KEY23_SIG,
    RELEASE_KEY24_SIG,
    
    END_OPERATION_SIG, //操作结束事件
    
	MAX_PUB_SIG,       /* the last published signal */

    TIMEOUT_SIG,       // 定时事件
    CONNECT_SIG,       // 连接事件
    DISCONNECT_SIG,    // 断开连接事件
    DATA_RECEIVED_SIG, // 接收到网络数据集事件
    
    COMMAND_OPERATION_SIG, //操作命令事件
    RESULT_OPERATION_SIG,  //操作结果事件
    
    SYN_TIME_SIG,          //同步时间
    ALARM_SIG,             //告警
    DISALARM_SIG,          //取消告警
	
	MAX_SIG            /* the last signal (keep always last) */
};


//接收到网络数据事件
typedef struct {
    QEvt super;
    
    size_t len;  // 数据长度
    char data[MAX_DATARECV_SIZE]; // 指向数据的指针
}DataRecvEvt;

//操作命令事件
typedef struct{
    QEvt super;
 
    CMD_TYPE_T type;       //操作类型
    uint32_t   duration;   //操作时长(秒)
}CmdOperationEvt;

//按钮操作类型
typedef enum
{
    CLICK_BNT, //按
    OPEN_BNT,  //开
    CLOSE_BNT  //关
}op_bnt_t;

typedef struct
{
    uint8_t num_bnts;                 //已按下按钮个数
    uint8_t  seq_bnts[NUM_BUTTONS+1];  //按下顺序,[0]无效，下标i为按钮编号
    op_bnt_t op_bnts[NUM_BUTTONS+1];   //按钮操作类型，[0]无效，下标i为按钮编号
}RsltRecon_t;


//操作结果事件
typedef struct{
    QEvt super;
    
    //char *uuid;
    CMD_TYPE_T type;       //操作类型
    union
    {
        RsltRecon_t recon; //识别结果
        int8_t      torque;//扭矩值
    }value; 
}RsltOperationEvt;

//同步时间时间
typedef struct{
    QEvt super;
    uint32_t time;
}SynTimeEvt;

// 初始化操作活动对象
void Operater_ctor(void);
// 初始化网络通信活动对象
void NetClient_ctor(void);

/* opaque pointers to active objects in the application */
extern QActive * const AO_NetClient;
extern QActive *const AO_Operater;

#endif
