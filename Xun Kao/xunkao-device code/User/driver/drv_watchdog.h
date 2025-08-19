/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRV_WDG_H
#define DRV_WDG_H

/* includes ----------------------------------------------------------------- */
#include "elab/edf/normal/elab_watchdog.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct elab_watchdog_driver
{
    elab_watchdog_t device;
    
    IWDG_HandleTypeDef hiwdg;
} elab_watchdog_driver_t;

/* public functions --------------------------------------------------------- */
void elab_driver_watchdog_init(elab_watchdog_driver_t *me, const char *name);

#ifdef __cplusplus
}
#endif

#endif  /* DRV_PIN_H */

/* ----------------------------- end of file -------------------------------- */
