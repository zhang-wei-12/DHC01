
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab/common/elab_export.h"
#include "../driver/drv_pwm_in.h"
#include "elab/common/elab_assert.h"

/* private variables -------------------------------------------------------- */
static elab_pwm_in_driver_t pwm_in_drv;

/* public functions --------------------------------------------------------- */
static void pwm_in_init_export(void)
{
    elab_driver_pwm_in_init(&pwm_in_drv, "PWM_IN", "B.06");
}
INIT_EXPORT(pwm_in_init_export, EXPORT_DRVIVER);

/* ----------------------------- end of file -------------------------------- */
