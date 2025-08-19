
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "../driver/drv_pin.h"
#include "elab/common/elab_export.h"
#include "export.h"

/* private variables -------------------------------------------------------- */
//static elab_pin_driver_t pin_mcu_led_R_H10;
//static elab_pin_driver_t pin_mcu_led_G_H11;
//static elab_pin_driver_t pin_mcu_led_B_H12;

//static elab_pin_driver_t pin_mcu_button_KEY1_A00;
//static elab_pin_driver_t pin_mcu_button_KEY2_C13;
//static elab_pin_driver_t pin_rs485_tx_en;

static elab_pin_driver_t pin_mcu_led;

static elab_pin_driver_t _pin_KEY_IN[MAX_KEY_IN] = {0};
//输入按键资源配置
pin_key_src_t _pin_key_src[MAX_KEY_IN] = 
{
    {PRESS_BNT, "1", "KEY_IN1", "F.06"},
    {PRESS_BNT, "2", "KEY_IN2", "F.07"},
    {PRESS_BNT, "3", "KEY_IN3", "F.08"},
    {PRESS_BNT, "4", "KEY_IN4", "F.09"},
    {PRESS_BNT, "5", "KEY_IN5", "F.10"},
    {PRESS_BNT, "6", "KEY_IN6", "G.06"},
    {PRESS_BNT, "7", "KEY_IN7", "G.07"},
    {PRESS_BNT, "8", "KEY_IN8", "G.09"},
    {ROTATE_BNT, "9", "KEY_IN9", "G.10"},
    {PRESS_BNT, "10", "KEY_IN10", "G.11"},
    {PRESS_BNT, "11", "KEY_IN11", "G.12"},
    {PRESS_BNT, "12", "KEY_IN12", "I.00"},
    {PRESS_BNT, "13", "KEY_IN13", "I.01"},
    {PRESS_BNT, "14", "KEY_IN14", "H.05"},
    {PRESS_BNT, "15", "KEY_IN15", "H.06"},
    {ROTATE_BNT, "16", "KEY_IN16", "H.07"},
    {PRESS_BNT, "17", "KEY_IN17", "H.08"},
    {PRESS_BNT, "18", "KEY_IN18", "H.09"},
    {PRESS_BNT, "19", "KEY_IN19", "H.10"},
    {PRESS_BNT, "20", "KEY_IN20", "H.11"},
    {ROTATE_BNT, "21","KEY_IN21", "I.02"},
    {PRESS_BNT, "22","KEY_IN22", "I.03"},
    {PRESS_BNT, "23","KEY_IN23", "I.04"},
    {PRESS_BNT, "24","KEY_IN24", "I.05"},
};

/* public functions --------------------------------------------------------- */
static void driver_pin_mcu_export(void)
{
    /* PIN devices on MCU. */
    /*
    elab_driver_pin_init(&pin_mcu_led_R_H10, "pin_led_R", "H.10");
    elab_pin_set_mode(&pin_mcu_led_R_H10.device.super, PIN_MODE_OUTPUT_PP);
    
    elab_driver_pin_init(&pin_mcu_led_G_H11, "pin_led_G", "H.11");
    elab_pin_set_mode(&pin_mcu_led_G_H11.device.super, PIN_MODE_OUTPUT_PP);
    
    elab_driver_pin_init(&pin_mcu_led_B_H12, "pin_led_B", "H.12");
    elab_pin_set_mode(&pin_mcu_led_B_H12.device.super, PIN_MODE_OUTPUT_PP);
    
    elab_driver_pin_init(&pin_mcu_button_KEY1_A00, "pin_button_KEY1", "A.00");
    elab_pin_set_mode(&pin_mcu_button_KEY1_A00.device.super, PIN_MODE_INPUT);
    
    elab_driver_pin_init(&pin_mcu_button_KEY2_C13, "pin_button_KEY2", "C.13");
    elab_pin_set_mode(&pin_mcu_button_KEY2_C13.device.super, PIN_MODE_INPUT);
    */
    elab_driver_pin_init(&pin_mcu_led, "pin_led", "A.08");
    elab_pin_set_mode(&pin_mcu_led.device.super, PIN_MODE_OUTPUT_PP);
    
    for(int i = 0; i < MAX_KEY_IN; i++)
    {
        elab_driver_pin_init(&_pin_KEY_IN[i], _pin_key_src[i].pin_dev, _pin_key_src[i].pin_name);
        elab_pin_set_mode(&_pin_KEY_IN[i].device.super, PIN_MODE_INPUT);
    }
    
    /*
    elab_driver_pin_init(&pin_rs485_tx_en, "pin_rs485_tx_en", "B.08");
    elab_pin_set_mode(&pin_rs485_tx_en.device.super, PIN_MODE_OUTPUT_PP);
    */
}
INIT_EXPORT(driver_pin_mcu_export, EXPORT_LEVEL_HW_INDEPNEDENT);
/* ----------------------------- end of file -------------------------------- */
