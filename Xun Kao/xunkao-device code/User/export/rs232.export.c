
/* includes ----------------------------------------------------------------- */
#include "../driver/drv_serial.h"
#include "elab/common/elab_export.h"
#include "elab/edf/normal/elab_serial.h"

static elab_serial_driver_t serial_rs232;

static void driver_rs232_export(void)
{
    static const elab_serial_config_t rs232_config = 
    {
        .baud_rate = 115200,
        .data_bits = ELAB_SERIAL_DATA_BITS_8,                
        .stop_bits = ELAB_SERIAL_STOP_BITS_1,                
        .parity = ELAB_SERIAL_PARITY_NONE,                
        .mode = ELAB_SERIAL_MODE_FULL_DUPLEX,                     
    };
    
    elab_driver_serial_init(&serial_rs232, "RS232", "UART6", rs232_config);
}
INIT_EXPORT(driver_rs232_export, EXPORT_DRVIVER);