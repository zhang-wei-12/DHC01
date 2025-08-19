/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_PWM_IN_H
#define __ELAB_PWM_IN_H

/* includes ----------------------------------------------------------------- */
#include "../elab_device.h"
#include "../../os/cmsis_os.h"
#include "../../elib/elib_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private types -----------------------------------------------------------  */

typedef struct elab_pwm_in
{
    elab_device_t super;
	const struct elab_pwm_in_ops *ops;
} elab_pwm_in_t;

typedef struct elab_pwm_in_ops
{
    float (* get_freq)(elab_pwm_in_t * const me);
	float (* get_duty)(elab_pwm_in_t * const me);
} elab_pwm_in_ops_t;

#define ELAB_ADC_CAST(_dev)             ((elab_pwm_in_t *)_dev)

/* public functions --------------------------------------------------------- */
void elab_pwm_in_register(elab_pwm_in_t * const me, const char *name,
                          const elab_pwm_in_ops_t *ops, void *user_data);

float elab_pwm_in_freq(elab_device_t *const me);
float elab_pwm_in_duty(elab_device_t *const me);

#ifdef __cplusplus
}
#endif

#endif  

/* ----------------------------- end of file -------------------------------- */
