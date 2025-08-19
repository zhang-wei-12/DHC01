
/* includes ----------------------------------------------------------------- */
#include "../driver/drv_pin.h"
#include "../driver/drv_serial.h"
#include "elab/common/elab_export.h"
#include "elab/edf/normal/elab_serial.h"

static elab_serial_driver_t serial_rs485;
static elab_pin_driver_t pin_rs485_tx_en;

static void driver_rs485_export(void)
{
    static const elab_serial_config_t rs485_config = 
    {
        .baud_rate = 115200,
        .data_bits = ELAB_SERIAL_DATA_BITS_8,                
        .stop_bits = ELAB_SERIAL_STOP_BITS_1,                
        .parity = ELAB_SERIAL_PARITY_NONE,                
        .mode = ELAB_SERIAL_MODE_HALF_DUPLEX,                     
    };
    
    //注册发送使能引脚
    elab_driver_pin_init(&pin_rs485_tx_en, "pin_rs485_tx_en", "B.08");
    elab_pin_set_mode(&pin_rs485_tx_en.device.super, PIN_MODE_OUTPUT_PP);
    //注册RS485串口设备
    elab_driver_serial_init(&serial_rs485, "RS485", "UART2", rs485_config);
     
}
INIT_EXPORT(driver_rs485_export, EXPORT_DRVIVER);
