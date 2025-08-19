/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRV_RTC_H
#define DRV_RTC_H

/* includes ----------------------------------------------------------------- */
#include "elab/edf/normal/elab_rtc.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct elab_rtc_driver
{
    elab_rtc_t device;
    
    RTC_HandleTypeDef hrtc;
} elab_rtc_driver_t;

/* public functions --------------------------------------------------------- */
void elab_driver_rtc_init(elab_rtc_driver_t *me, const char *name);

#ifdef __cplusplus
}
#endif

#endif  /* DRV_PIN_H */

/* ----------------------------- end of file -------------------------------- */
