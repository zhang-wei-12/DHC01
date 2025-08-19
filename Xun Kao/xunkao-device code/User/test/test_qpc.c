/* includes ----------------------------------------------------------------- */
#include "elab/3rd/qpc/include/qpc.h"
#include "elab/os/cmsis_os.h"
#include "elab/common/elab_export.h"
#include "elab/common/elab_assert.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/edf/elab_device.h"
#include "elab/edf/user/elab_button.h"
#include "elab/edf/user/elab_led.h"
#include "config/event_def.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("TestQpc");
Q_DEFINE_THIS_MODULE("TestQpc")

typedef struct QA_bnt_key
{
    QActive super;
    elab_device_t *led_r;
    elab_device_t *led_g;
    elab_device_t *led_b;
}QA_bnt_key_t;

/* private state function prototypes -----------------------------------------*/
static QState _state_initial(QA_bnt_key_t * const me, void const * const pre);
static QState _state_idle(QA_bnt_key_t * const me, QEvt const * const e);
static QState _state_work(QA_bnt_key_t * const me, QEvt const * const e);
static QState _state_red(QA_bnt_key_t * const me, QEvt const * const e);
static QState _state_blue(QA_bnt_key_t * const me, QEvt const * const e);
static QState _state_green(QA_bnt_key_t * const me, QEvt const * const e);

/* Private variables ---------------------------------------------------------*/
static QA_bnt_key_t OA_bnt_key;

static const char *str_info[] =
{
    "EVT_BUTTON_KEY1.",
    "EVT_BUTTON_KEY2.",
};

/* public functions --------------------------------------------------------- */
static void test_qpc_init(void)
{
    static uint8_t stack[1024];                             /* stack. */
    static QEvt const *e_queue[8];                          /* event queue. */

    QActive_ctor(&OA_bnt_key.super, Q_STATE_CAST(&_state_initial));
    QACTIVE_START(&OA_bnt_key.super,
                    osPriorityLow,                  /* QP priority */
                    e_queue, Q_DIM(e_queue),        /* evt queue */
                    (void *)stack, 1024U,           /* thread stack */
                    (QEvt *)0);                     /* no initialization event */
}
//INIT_EXPORT(test_qpc_init, EXPORT_APP);

/* private state function ----------------------------------------------------*/
static QState _state_initial(QA_bnt_key_t * const me, void const * const pre)
{
    (void)pre;
    (void)me;

    elab_device_t *dev = NULL;

    QActive_subscribe(&me->super, EVT_BUTTON_KEY1);
    QActive_subscribe(&me->super, EVT_BUTTON_KEY2);

    /* Button start click event. */
    dev = elab_device_find("button_KEY1");
    elab_assert(dev != NULL);
    elab_button_set_event_signal(dev, ELAB_BUTTON_EVT_CLICK, EVT_BUTTON_KEY1);
    
    dev = elab_device_find("button_KEY2");
    elab_assert(dev != NULL);
    elab_button_set_event_signal(dev, ELAB_BUTTON_EVT_CLICK, EVT_BUTTON_KEY2);

    dev = elab_device_find("led_R");
    elab_assert(dev != NULL);
    me->led_r = dev;
    
    dev = elab_device_find("led_G");
    elab_assert(dev != NULL);
    me->led_g = dev;
    
    dev = elab_device_find("led_B");
    elab_assert(dev != NULL);
    me->led_b = dev;
    
    return Q_TRAN(&_state_idle);
}

static QState _state_idle(QA_bnt_key_t * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&QHsm_top);

    switch (e->sig)
    {
    case EVT_BUTTON_KEY1:
        elog_debug(str_info[e->sig - EVT_BUTTON_KEY1]);
        ret = Q_TRAN(&_state_work);
        break;

    case EVT_BUTTON_KEY2:
        elog_debug(str_info[e->sig - EVT_BUTTON_KEY1]);
        ret = Q_HANDLED();
        break;
    }
    
    return ret;
}

static QState _state_work(QA_bnt_key_t * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&QHsm_top);

    switch (e->sig)
    {
    case Q_INIT_SIG:
        ret = Q_TRAN(&_state_red);
        break;
    
    case EVT_BUTTON_KEY2:
        elog_debug(str_info[e->sig - EVT_BUTTON_KEY1]);
        ret = Q_TRAN(&_state_idle);
        break;
    }

    return ret;
}

static QState _state_red(QA_bnt_key_t * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&_state_work);

    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        //ºìµÆÉÁË¸Æô¶¯
        elab_led_toggle(me->led_r, 500);
        ret = Q_HANDLED();
        break;
    
    case EVT_BUTTON_KEY1:
        elog_debug(str_info[e->sig - EVT_BUTTON_KEY1]);
        ret = Q_TRAN(&_state_blue);
        break;

    case Q_EXIT_SIG:
        //ºìµÆÉÁË¸Í£Ö¹;
        elab_led_clear(me->led_r);
        ret = Q_HANDLED();
        break;
    }

    return ret;
}

static QState _state_blue(QA_bnt_key_t * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&_state_work);

    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        //À¶µÆÉÁË¸Æô¶¯
        elab_led_toggle(me->led_b, 500);
        ret = Q_HANDLED();
        break;
    
    case EVT_BUTTON_KEY1:
        elog_debug(str_info[e->sig - EVT_BUTTON_KEY1]);
        ret = Q_TRAN(&_state_green);
        break;

    case Q_EXIT_SIG:
        //À¶µÆÉÁË¸Í£Ö¹;
        elab_led_clear(me->led_b);
        ret = Q_HANDLED();
        break;
    }

    return ret;
}

static QState _state_green(QA_bnt_key_t * const me, QEvt const * const e)
{
    QState ret = Q_SUPER(&_state_work);

    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        //ÂÌµÆÉÁË¸Æô¶¯
        elab_led_toggle(me->led_g, 500);
        ret = Q_HANDLED();
        break;
    
    case EVT_BUTTON_KEY1:
        elog_debug(str_info[e->sig - EVT_BUTTON_KEY1]);
        ret = Q_TRAN(&_state_red);
        break;

    case Q_EXIT_SIG:
        //ÂÌµÆÉÁË¸Í£Ö¹;
        elab_led_clear(me->led_g);
        ret = Q_HANDLED();
        break;
    }

    return ret;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
