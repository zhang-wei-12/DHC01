
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_pwm_in.h"
#include "../../common/elab_assert.h"

ELAB_TAG("Edf_ADC");

/* private variables -------------------------------------------------------- */

static elab_dev_ops_t _ops =
{
    .enable = NULL,
    .read = NULL,
    .write = NULL,
};

/* public functions --------------------------------------------------------- */
/**
  * @brief  eLab pwm in register function.
  * @param  me          this pointer
  * @param  name        pwm_in's name.
  * @param  ops         ops interface.
  * @param  user_data   User private data.
  * @retval None
  */
void elab_pwm_in_register(elab_pwm_in_t * const me, const char *name,
                        const elab_pwm_in_ops_t *ops, void *user_data)
{
    assert(me != NULL);
    assert(name != NULL);
    assert(ops != NULL);

    elab_device_attr_t attr =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_PWM_IN,
    };
    elab_device_register(&me->super, &attr);
    me->super.user_data = user_data;
    me->super.ops = &_ops;

    me->ops = ops;
}
/**
  * @brief  Get the eLab pwm in frequency.
  * @param  me          this pointer
  * @retval freq        The frequency
  */
float elab_pwm_in_freq(elab_device_t *const me)
{
    assert(me != NULL);
    assert(me->attr.type == ELAB_DEVICE_PWM_IN);
    
    elab_pwm_in_t *pwm = (elab_pwm_in_t *)me;
    
    return pwm->ops->get_freq(pwm);
}
/**
  * @brief  Get the eLab pwm in duty.
  * @param  me          this pointer
  * @retval duty        The duty
  */
float elab_pwm_in_duty(elab_device_t *const me)
{
    assert(me != NULL);
    assert(me->attr.type == ELAB_DEVICE_PWM_IN);
    
    elab_pwm_in_t *pwm = (elab_pwm_in_t *)me;
    
    return pwm->ops->get_duty(pwm);
}

/* ----------------------------- end of file -------------------------------- */
