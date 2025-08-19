#include "app.h"
#include "common/elab_assert.h"
#include "elab/common/elab_log.h"
#include "elab/edf/elab_device.h"
#include "elab/edf/user/elab_button.h"
#include "elab/edf/normal/elab_pwm_in.h"

#include "../driver/drv_button.h"
#include "../driver/drv_pin.h"
#include "../driver/drv_rtc.h"
#include <time.h>

ELAB_TAG("OPERATER");
Q_DEFINE_THIS_MODULE("OPERATER")

//引用按钮设备
extern pin_key_src_t _pin_key_src[MAX_KEY_IN];

#define IS_VALID_FREQ(freq)  (freq >= 5000 && freq <= 15000)
    
#define CAL_TORQUE(freq, torque) \
do{ \
     if (freq >= 10000) \
        torque = (freq - 10000) / 5000.0 * 100.0 + 0.5; \
    else if (freq >= 5000) \
        torque = (10000 - freq) / 5000.0 * (-100.0) - 0.5;\
}while(0) 

//定义一个活动对象来管理操作器
typedef struct operater{
    QActive super;
    
    //char *uuid;
    
    uint8_t num_bnts;                  //按下按钮总数
    uint8_t seq_bnts[NUM_BUTTONS+1];   //按下按钮顺序,[0]无效，[i]对应按编号按钮按下顺序
    op_bnt_t op_bnts[NUM_BUTTONS+1];   //按钮操作类型,[0]无效，[i]对应按编号按钮按下顺序
    uint8_t press_times;               //按下次数
    
    int8_t      torque;                 //扭矩值
    
    //设备句柄
    elab_device_t *rtc;                  //实时时钟
    elab_device_t *buttons[NUM_BUTTONS]; //识别按钮
    elab_device_t *pwm_in;               //pwm输入
    
    QTimeEvt timer;                     //操作定时器 
}Operater;

static QState Operater_button(Operater * const me, QEvt const * const e, int type);

//初始化
static QState Operater_initial(Operater * const me, void const * const pre);
static QState Operater_idle(Operater * const me, QEvt const * const e);
static QState Operater_recongnition(Operater * const me, QEvt const * const e);
static QState Operater_assembly(Operater * const me, QEvt const * const e);
static QState Operater_rest(Operater * const me, QEvt const * const e);

static Operater l_operater;//定义Operater对象
QActive *const AO_Operater = &l_operater.super;//定义Operater活动对象指针

// 初始化操作活动对象
void Operater_ctor(void) {
	Operater *me = &l_operater;
    
    QActive_ctor(&me->super, Q_STATE_CAST(&Operater_initial));
    QTimeEvt_ctorX(&me->timer, AO_Operater, TIMEOUT_SIG, 0);  //创建定时器
}

//实现状态机: 初始化
static QState Operater_initial(Operater * const me, void const * const pre) {
    (void)pre;
    
    //QActive_subscribe(&me->super, BUTTON_KEY1_SIG);
    //QActive_subscribe(&me->super, BUTTON_KEY2_SIG);
    
    elab_device_t *dev = NULL; 
    
    
    dev = elab_device_find("rtc");
    elab_assert(dev != NULL);
    me->rtc = dev;
    
    //按钮设备
    for(int i = 0; i < MAX_KEY_IN; i++)
    {
        QActive_subscribe(&me->super, BUTTON_KEY1_SIG + i);
        QActive_subscribe(&me->super, RELEASE_KEY1_SIG + i);
        
        QActive_subscribe(&me->super, END_OPERATION_SIG);
        
        dev = elab_device_find(_pin_key_src[i].key_no);
        elab_assert(dev != NULL);
        me->buttons[i] = dev;
        
        if (_pin_key_src[i].type == PRESS_BNT)
        {
            elab_button_set_event_signal(dev, ELAB_BUTTON_EVT_PRESSED, BUTTON_KEY1_SIG + i);
        }
        else if (_pin_key_src[i].type == ROTATE_BNT)
        {
            elab_button_set_event_signal(dev, ELAB_BUTTON_EVT_PRESSED, BUTTON_KEY1_SIG + i);
            elab_button_set_event_signal(dev, ELAB_BUTTON_EVT_RELEASE, RELEASE_KEY1_SIG + i);     
        }
    }
    
    dev = elab_device_find("PWM_IN");
    elab_assert(dev != NULL);
    me->pwm_in = dev;
    
    return Q_TRAN(&Operater_idle);
}

//实现状态机: 空闲状态
static QState Operater_idle(Operater * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&QHsm_top);
    
    switch (e->sig)
    {
        case SYN_TIME_SIG:{
            //TODO: 同步时间
            SynTimeEvt *SynTmEvt = (SynTimeEvt *)e;
            time_t rawtime = (time_t)SynTmEvt->time;
            //elog_debug("Operater_idle: SYN_TIME_SIG is %d!\n", rawtime);
            struct tm * timeinfo = localtime(&rawtime);
            
            elab_rtc_time_t rt = 
            {
                .date = {.year = timeinfo->tm_year + 1900, .month = timeinfo->tm_mon + 1, .day = timeinfo->tm_mday},
                .time = {.hour = timeinfo->tm_hour, .minute = timeinfo->tm_min, .second = timeinfo->tm_sec},
            };
        
            elab_rtc_set_time(me->rtc, &rt);
            
            ret = Q_HANDLED();
            break;
        }
        
        case ALARM_SIG:
            ;//TODO：告警
            ret = Q_HANDLED();
         break;
        
        case DISALARM_SIG:
            ;//TODO：取消告警
            ret = Q_HANDLED();
         break;
        
        case COMMAND_OPERATION_SIG:{
            //elog_debug("Operater_idle: COMMAND_OPERATION_SIG!\n");
            CmdOperationEvt *CmdOpEvt = (CmdOperationEvt *)e;      
            
            QTimeEvt_armX(&me->timer, 1000 * CmdOpEvt->duration, 0);
        
            if (CmdOpEvt->type == CMD_TYPE_RECOGNITION)
                ret = Q_TRAN(&Operater_recongnition);
            else if (CmdOpEvt->type == CMD_TYPE_ASSEMBLY)
                ret = Q_TRAN(&Operater_assembly);
            else if (CmdOpEvt->type == CMD_TYPE_RESET)
                ret = Q_TRAN(&Operater_rest);
            break; 
        }
    }
    
    return ret;
}

static QState Operater_button(Operater * const me, QEvt const * const e, int type)
{
     QState ret = Q_SUPER(&QHsm_top);
    
    switch (e->sig) 
    {   
        case Q_ENTRY_SIG:
        {
            //elog_debug("Operater_button: Q_ENTRY_SIG!\n");
            me->num_bnts = 0;
            memset(me->seq_bnts, 0, sizeof(me->seq_bnts));
            memset(me->op_bnts, 0, sizeof(me->op_bnts));
            me->press_times = 0;
            ret = Q_HANDLED();
            break;
        }
        case END_OPERATION_SIG:
        {
            //elog_debug("Operater_button: Receive END_OPERATION_SIG !"); 
            QTimeEvt_disarm(&me->timer);//解除已有定时器
            
            //不加break，执行下面的TIMEOUT_SIG分之
        }
        case TIMEOUT_SIG:
        {
            //elog_debug("Operater_button: TIMEOUT_SIG!\n");
            //TODO:向网络对象发送操作结果RESULT_OPERATION_SIG
            //elog_debug("me->num_bnts is %d!\n", me->num_bnts);
        
            RsltOperationEvt *RsltOpEvt = Q_NEW(RsltOperationEvt, RESULT_OPERATION_SIG);
            RsltOpEvt->type = type;
            RsltOpEvt->value.recon.num_bnts = me->num_bnts;
            memcpy(&RsltOpEvt->value.recon.seq_bnts, &me->seq_bnts, sizeof(me->seq_bnts)); 
            memcpy(&RsltOpEvt->value.recon.op_bnts, &me->op_bnts, sizeof(me->op_bnts)); 
        
            QACTIVE_POST(AO_NetClient, (QEvt *)RsltOpEvt, 0);
          
            ret = Q_TRAN(&Operater_idle);
            break;
        }
        case BUTTON_KEY1_SIG:
        case BUTTON_KEY2_SIG:
        case BUTTON_KEY3_SIG:
        case BUTTON_KEY4_SIG:
        case BUTTON_KEY5_SIG:
        case BUTTON_KEY6_SIG:
        case BUTTON_KEY7_SIG:
        case BUTTON_KEY8_SIG:
        case BUTTON_KEY9_SIG:
        case BUTTON_KEY10_SIG:
        case BUTTON_KEY11_SIG:
        case BUTTON_KEY12_SIG:
        case BUTTON_KEY13_SIG:
        case BUTTON_KEY14_SIG:
        case BUTTON_KEY15_SIG:
        case BUTTON_KEY16_SIG:
        case BUTTON_KEY17_SIG:
        case BUTTON_KEY18_SIG:
        case BUTTON_KEY19_SIG:
        case BUTTON_KEY20_SIG:
        case BUTTON_KEY21_SIG:
        case BUTTON_KEY22_SIG:
        case BUTTON_KEY23_SIG:
        case BUTTON_KEY24_SIG:
        {   
            uint8_t id = e->sig - BUTTON_KEY1_SIG + 1;
            elog_debug("Operater_button: pressed bnt is %d!\n", id);
            if (me->seq_bnts[id] == 0)
            {
                me->num_bnts++;
            }
            me->press_times++;
            me->seq_bnts[id] = me->press_times;//记录id对应按钮操作的顺序
            
            if (_pin_key_src[id - 1].type == PRESS_BNT)
            {
                me->op_bnts[id] = CLICK_BNT;       //按钮按下
            }
            else if (_pin_key_src[id - 1].type == ROTATE_BNT) 
            {
                me->op_bnts[id] = CLOSE_BNT;       //旋钮关
            }
            
            //elog_debug("Press: id is %d, op is %d\n", id, me->op_bnts[id]);  
                   
            ret = Q_HANDLED();
            break;
        }
        case RELEASE_KEY1_SIG:
        case RELEASE_KEY2_SIG:
        case RELEASE_KEY3_SIG:
        case RELEASE_KEY4_SIG:
        case RELEASE_KEY5_SIG:
        case RELEASE_KEY6_SIG:
        case RELEASE_KEY7_SIG:
        case RELEASE_KEY8_SIG:
        case RELEASE_KEY9_SIG:
        case RELEASE_KEY10_SIG:
        case RELEASE_KEY11_SIG:
        case RELEASE_KEY12_SIG:
        case RELEASE_KEY13_SIG:
        case RELEASE_KEY14_SIG:
        case RELEASE_KEY15_SIG:
        case RELEASE_KEY16_SIG:
        case RELEASE_KEY17_SIG:
        case RELEASE_KEY18_SIG:
        case RELEASE_KEY19_SIG:
        case RELEASE_KEY20_SIG:
        case RELEASE_KEY21_SIG:
        case RELEASE_KEY22_SIG:
        case RELEASE_KEY23_SIG:
        case RELEASE_KEY24_SIG:
        {   
            uint8_t id = e->sig - RELEASE_KEY1_SIG + 1;
            elog_debug("Operater_button: release bnt is %d!\n", id);
            if (me->seq_bnts[id] == 0)
            {
                me->num_bnts++;
            }
            me->press_times++;
            me->seq_bnts[id] = me->press_times;//记录id对应按钮操作的顺序
            
            me->op_bnts[id] = OPEN_BNT;       //旋钮开
            //elog_debug("Release: id is %d, op is %d\n", id, me->op_bnts[id]);  
                   
            ret = Q_HANDLED();
            break;
        }
    }
    
    return ret;
}

//实现状态机: 复位操作
static QState Operater_rest(Operater * const me, QEvt const * const e) {
    return Operater_button(me, e, CMD_TYPE_RESET);
}

//实现状态机: 识别操作
static QState Operater_recongnition(Operater * const me, QEvt const * const e) { 
   return Operater_button(me, e, CMD_TYPE_RECOGNITION);
}

static QState Operater_assembly(Operater * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&QHsm_top);
    
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
            //elog_debug("Operater_alignment: Q_ENTRY_SIG!\n");
            me->torque = 0;
   
            ret = Q_HANDLED();
            break;
        case END_OPERATION_SIG:
        {
            elog_debug("Operater_assembly: Receive END_OPERATION_SIG !"); 
            QTimeEvt_disarm(&me->timer);//解除已有定时器
            
            //不加break，执行下面的TIMEOUT_SIG分之
        }
        case TIMEOUT_SIG:
        {
            elog_debug("Operater_alignment: TIMEOUT_SIG!\n");
            //TODO:向网络对象发送操作结果RESULT_OPERATION_SIG
            int freq = (int)(elab_pwm_in_freq(me->pwm_in) + 0.5);
            elog_debug("Operater_alignment: freq is %d!\n", freq);
            
            if (!IS_VALID_FREQ(freq))
            {
                freq = 10000;
            }
         
            CAL_TORQUE(freq, me->torque);
            elog_debug("Operater_alignment: torque is %d!\n", me->torque);
            
            RsltOperationEvt *RsltOpEvt = Q_NEW(RsltOperationEvt, RESULT_OPERATION_SIG);
            RsltOpEvt->type = CMD_TYPE_ASSEMBLY;
            RsltOpEvt->value.torque = me->torque;
        
            QACTIVE_POST(AO_NetClient, (QEvt *)RsltOpEvt, 0);
            ret = Q_TRAN(&Operater_idle);
            break;
        }
    }
    
    return ret;
}

