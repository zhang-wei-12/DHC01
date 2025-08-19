
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab/common/elab_export.h"
#include "../driver/drv_rtc.h"
#include "elab/common/elab_assert.h"

ELAB_TAG("RTC_EXPORT");

/* private variables -------------------------------------------------------- */
static elab_rtc_driver_t rtc_drv;

/* public functions --------------------------------------------------------- */
static void rtc_init_export(void)
{
    elab_driver_rtc_init(&rtc_drv, "rtc");
    
    elab_device_t *dev = elab_device_find("rtc");
    elab_assert(dev != NULL);
    
    elab_rtc_time_t rt = 
    {
        .date = {.year = 2024, .month = 11, .day = 25},
        .time = {.hour = 16, .minute = 30, .second = 0},
    };
        
    elab_rtc_set_time(dev, &rt);
}
INIT_EXPORT(rtc_init_export, EXPORT_DRVIVER);

/* ----------------------------- end of file -------------------------------- */
