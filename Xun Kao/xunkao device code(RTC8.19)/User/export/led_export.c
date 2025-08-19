
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab/common/elab_export.h"
#include "elab/edf/user/elab_led.h"
#include "export.h"

/* private variables -------------------------------------------------------- */
/*
static elab_led_t led_R;
static elab_led_t led_G;
static elab_led_t led_B;
*/
static elab_led_t led_run;

/* public functions --------------------------------------------------------- */
static void led_export(void)
{
    /* PIN devices on MCU. */
    /*
    elab_led_register(&led_R, "led_R", "pin_led_R", false);
    elab_led_register(&led_G, "led_G", "pin_led_G", false);
    elab_led_register(&led_B, "led_B", "pin_led_B", false);
    */
    elab_led_register(&led_run, "led_run", "pin_led", true);
}
INIT_EXPORT(led_export, EXPORT_DRVIVER);

/* ----------------------------- end of file -------------------------------- */
