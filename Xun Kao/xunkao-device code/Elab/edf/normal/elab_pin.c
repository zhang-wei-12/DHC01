
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "elab_pin.h"
#include "../../common/elab_assert.h"

ELAB_TAG("Edf_Pin");

#ifdef __cplusplus
extern "C" {
#endif

/* public functions --------------------------------------------------------- */
/**
  * @brief  eLab pin initialization.
  * @param  me      this pointer
  * @param  name    pin's name.
  * @param  mode    pin's mode.
  * @retval None
  */
void elab_pin_register(elab_pin_t * const me,
                        const char *name,
                        const elab_pin_ops_t *ops,
                        void *user_data)
{
    assert(me != NULL);
    assert(name != NULL);
    assert(ops != NULL);

    elab_device_attr_t attr =
    {
        .name = (char *)name,
        .sole = true,
        .type = ELAB_DEVICE_PIN,
    };
    elab_device_register(&me->super, &attr);
    me->super.user_data = user_data;
    me->super.ops = NULL;

    me->ops = ops;
    me->ops->init(me);
    me->mode = PIN_MODE_MAX;
    elab_err_t ret = me->ops->get_status(me, &me->status);
    elab_assert(ret == ELAB_OK);
}

/**
  * @brief  eLab pin's status getting function.
  * @param  me      this pointer
  * @retval The pin's status.
  */
void elab_pin_set_mode(elab_device_t * const me, uint8_t mode)
{
    assert(me != NULL);

    elab_pin_t *pin = (elab_pin_t *)me;
    if (pin->mode != mode)
    {
        pin->ops->set_mode(pin, mode);
        pin->mode = mode;
    }
}


/**
  * @brief  eLab pin's status getting function.
  * @param  me      this pointer
  * @retval The pin's status.
  */
bool elab_pin_get_status(elab_device_t *const me)
{
    assert(me != NULL);
    assert(ELAB_PIN_CAST(me)->mode < PIN_MODE_MAX);

    elab_pin_t *pin = (elab_pin_t *)me;

    if (pin->mode <= PIN_MODE_INPUT_PULLDOWN)//输出状态下，引脚不会被外部改变，直接使用pin->status即可
    {
        bool status;
        elab_err_t ret = pin->ops->get_status(pin, &status);
        if (ret == ELAB_OK)
        {
            pin->status = status;
        }
    }
    
    return pin->status;
}

/**
  * @brief  eLab pin's status turning on function.
  * @param  me      this pointer
  * @param  status  the input pin status.
  * @retval None.
  */
void elab_pin_set_status(elab_device_t *const me, bool status)
{
    assert(me != NULL);
    assert_name(ELAB_PIN_CAST(me)->mode == PIN_MODE_OUTPUT_PP ||
                ELAB_PIN_CAST(me)->mode == PIN_MODE_OUTPUT_OD,
                me->attr.name);

    elab_pin_t *pin = ELAB_PIN_CAST(me);
    if (status != pin->status)
    {
        elab_err_t ret = pin->ops->set_status(pin, status);
        if (ret == ELAB_OK)
        {
            pin->status = status;
        }
    }
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
