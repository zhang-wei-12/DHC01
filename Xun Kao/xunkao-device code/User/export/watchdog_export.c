
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab/common/elab_export.h"
#include "../driver/drv_watchdog.h"
#include "elab/common/elab_assert.h"

ELAB_TAG("WatchdogExport");

/* private variables -------------------------------------------------------- */
static elab_watchdog_driver_t watchdog;

/* public functions --------------------------------------------------------- */
static void watchdog_init_export(void)
{
    elab_driver_watchdog_init(&watchdog, "iwdg");
    
    elab_device_t *dev = elab_device_find("iwdg");
    elab_assert(dev != NULL);
    
    elab_watchdog_set_time(dev, 6000);
    
}
//INIT_EXPORT(watchdog_init_export, EXPORT_DRVIVER);

static void watchdog_feed_export(void)
{
    elab_device_t *dev = elab_device_find("iwdg");
    elab_assert(dev != NULL);
    
    elab_watchdog_feed(dev);
}
//POLL_EXPORT(watchdog_feed_export, 3000);

/* ----------------------------- end of file -------------------------------- */
