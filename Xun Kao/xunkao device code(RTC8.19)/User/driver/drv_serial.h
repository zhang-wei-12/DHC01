
#ifndef DRV_SERIAL_H
#define DRV_SERIAL_H

/* includes ----------------------------------------------------------------- */
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dma.h"
#include "elab/edf/normal/elab_serial.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct elab_serial_driver
{
    elab_serial_t device;
    
    const char *name;
    elab_serial_config_t config;
    
    UART_HandleTypeDef huart;
    DMA_HandleTypeDef  hdma_uart_tx;
    DMA_HandleTypeDef  hdma_uart_rx;
    uint8_t            buffer_rx;
} elab_serial_driver_t;

/* public functions --------------------------------------------------------- */
void elab_driver_uart_init(elab_serial_driver_t *me, const char *name, const elab_serial_config_t config);

#ifdef __cplusplus
}
#endif

#endif
