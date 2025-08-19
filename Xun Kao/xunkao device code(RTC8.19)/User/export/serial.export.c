
/* includes ----------------------------------------------------------------- */
#include "../driver/drv_pin.h"
#include "../driver/drv_serial.h"
#include "elab/common/elab_export.h"
#include "elab/edf/normal/elab_serial.h"
#include "elab/edf/user/elab_RSxxx.h"

static RSxxx_t rs485_uart2, rs232_uart6, rs232_uart1;
static elab_serial_driver_t serial_uart6;
static elab_serial_driver_t serial_uart2;
static elab_serial_driver_t serial_uart1;

RSxxx_t* get_RSxxx_handle(const char *name)
{
    RSxxx_t *handle = NULL;
    
    if (strcmp(name, "RS232_UART1") == 0)
        handle = &rs232_uart1;
    else if (strcmp(name, "RS232_UART6") == 0)
        handle = &rs232_uart6;
    else if (strcmp(name, "RS232_UART6") == 0)
        handle = &rs485_uart2; 
    
    if(handle->serial != NULL)
        return handle;
    else 
        return NULL;
}

static void driver_rs232_uart1_export(void)
{
    static const elab_serial_config_t rs232_config = 
    {
        .baud_rate = 115200,
        .data_bits = ELAB_SERIAL_DATA_BITS_8,                
        .stop_bits = ELAB_SERIAL_STOP_BITS_1,                
        .parity = ELAB_SERIAL_PARITY_NONE, 
        .mode = ELAB_SERIAL_MODE_FULL_DUPLEX
    };
    
	//使用uart6
    //uart6_int(rs232_config);
    elab_driver_uart_init(&serial_uart1, "UART1", rs232_config);
    //初始化RS232 Handeler
    RSxxx_init(&rs232_uart1,"UART1", NULL, true, NULL);
}
//INIT_EXPORT(driver_rs232_uart1_export, EXPORT_DRVIVER);//for test Uart dma dtiver

static void driver_rs232_uart6_export(void)
{
    static const elab_serial_config_t rs232_config = 
    {
        .baud_rate = 115200,
        .data_bits = ELAB_SERIAL_DATA_BITS_8,                
        .stop_bits = ELAB_SERIAL_STOP_BITS_1,                
        .parity = ELAB_SERIAL_PARITY_NONE, 
        .mode = ELAB_SERIAL_MODE_FULL_DUPLEX
    };
    
	//使用uart6
    //uart6_int(rs232_config);
    elab_driver_uart_init(&serial_uart6, "UART6", rs232_config);
    //初始化RS232 Handeler
    RSxxx_init(&rs232_uart6,"UART6", NULL, true, NULL);
}
//INIT_EXPORT(driver_rs232_uart6_export, EXPORT_DRVIVER);

static void driver_rs485_uart2_export(void)
{
    static const elab_serial_config_t rs485_config = 
    {
        .baud_rate = 115200,
        .data_bits = ELAB_SERIAL_DATA_BITS_8,                
        .stop_bits = ELAB_SERIAL_STOP_BITS_1,                
        .parity = ELAB_SERIAL_PARITY_NONE,     
        .mode = ELAB_SERIAL_MODE_HALF_DUPLEX
    };
    
    //使用uart2
    //uart2_int(rs485_config);
    elab_driver_uart_init(&serial_uart2, "UART2", rs485_config);
    //初始化RS485 Handeler
    RSxxx_init(&rs485_uart2,"UART2","pin_rs485_tx_en", true, NULL);
}
//INIT_EXPORT(driver_rs485_uart2_export, EXPORT_DRVIVER);
