/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRV_BUTTON_H
#define DRV_BUTTON_H

/* includes ----------------------------------------------------------------- */
#include "elab/edf/user/elab_button.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct elab_button_driver
{
    elab_button_t device;

    const char *dev_name;
    float voltage_min;
    float voltage_max;
    bool true_pressed;
} elab_button_driver_t;

/* public functions --------------------------------------------------------- */
void elab_driver_button_pin_init(elab_button_driver_t *me,
                                    const char *name,
                                    const char *dev_name,
                                    bool true_pressed);
/*
void elab_driver_button_adc_init(elab_button_driver_t *me,
                                    const char *name,
                                    const char *dev_name,
                                    float voltage_min,
                                    float voltage_max);
*/

#ifdef __cplusplus
}
#endif

#endif  /* DRV_BUTTON_H */

/* ----------------------------- end of file -------------------------------- */
