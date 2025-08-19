#ifndef ELAB_UART_DEBUG_H
#define ELAB_UART_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "../config/FreeRTOSConfig.h"

#define ELAB_DEBUG_UART_BUFFER_TX               (512)
#define ELAB_DEBUG_UART_BUFFER_RX               (16)

#define DEBUG_USART                             UART5//USART1 
#define DEBUG_USART_CLK_ENABLE()                __UART5_CLK_ENABLE()//__USART1_CLK_ENABLE();

#define DEBUG_USART_RX_GPIO_PORT                GPIOD//GPIOA
#define DEBUG_USART_RX_GPIO_CLK_ENABLE()        __GPIOD_CLK_ENABLE()//__GPIOA_CLK_ENABLE()
#define DEBUG_USART_RX_PIN                      GPIO_PIN_2//GPIO_PIN_10
#define DEBUG_USART_RX_AF                       GPIO_AF8_UART5//GPIO_AF7_USART1

#define DEBUG_USART_TX_GPIO_PORT                GPIOC//GPIOA
#define DEBUG_USART_TX_GPIO_CLK_ENABLE()        __GPIOC_CLK_ENABLE()//__GPIOA_CLK_ENABLE()
#define DEBUG_USART_TX_PIN                      GPIO_PIN_12//GPIO_PIN_9
#define DEBUG_USART_TX_AF                       GPIO_AF8_UART5//GPIO_AF7_USART1

#define DEBUG_USART_IRQ                 		UART5_IRQn//USART1_IRQn
#define DEBUG_USART_IRQHandler                  UART5_IRQHandler//USART1_IRQHandler

/* public functions --------------------------------------------------------- */
void elab_debug_uart_txCallback(void);
void elab_debug_uart_rxCallback(void);

#ifdef __cplusplus
}
#endif

#endif

/* ----------------------------- end of file -------------------------------- */
