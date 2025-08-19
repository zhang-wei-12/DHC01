
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "../driver/drv_pin.h"
#include "../driver/drv_button.h"
#include "elab/common/elab_export.h"

/* private variables -------------------------------------------------------- */
//static elab_button_driver_t button_pin_KEY1;
//static elab_button_driver_t button_pin_KEY2;

//引用按钮设备
extern pin_key_src_t _pin_key_src[MAX_KEY_IN];
static elab_button_driver_t _buttons[MAX_KEY_IN] = {0};

/* public functions --------------------------------------------------------- */
static void driver_button_export(void)
{
    /* Buttons based on PIN devices. */
    /*
    elab_driver_button_pin_init(&button_pin_KEY1,
                                "button_KEY1", "pin_button_KEY1", true);
    elab_driver_button_pin_init(&button_pin_KEY2,
                                "button_KEY2", "pin_button_KEY2", true);
    */
    for(int i = 0; i < MAX_KEY_IN; i++)
    {
        elab_driver_button_pin_init(&_buttons[i], 
                                    _pin_key_src[i].key_no, _pin_key_src[i].pin_dev, false);//false表示按下输入低电平
    }
}
INIT_EXPORT(driver_button_export, EXPORT_DRVIVER);

/* ----------------------------- end of file -------------------------------- */
