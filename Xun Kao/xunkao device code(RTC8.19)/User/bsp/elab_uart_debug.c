/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_uart_debug.h"
#include "elab/common/elab_common.h"
#include "elab/elib/elib_queue.h"

/* private variables -------------------------------------------------------- */
UART_HandleTypeDef huart;

static elib_queue_t queue_rx;
static uint8_t buffer_rx[ELAB_DEBUG_UART_BUFFER_RX];
static elib_queue_t queue_tx;
static uint8_t buffer_tx[ELAB_DEBUG_UART_BUFFER_TX];
static uint8_t byte_recv = 0;
static uint8_t byte_send = 0;

/* public functions --------------------------------------------------------- */
/**
  * @brief  Initialize the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
void elab_debug_uart_init(uint32_t baudrate)
{    
    huart.Instance = DEBUG_USART;
    huart.Init.BaudRate = baudrate;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&huart);
    HAL_UART_Receive_IT(&huart, &byte_recv, 1);//启动接收
    
    elib_queue_init(&queue_rx, buffer_rx, ELAB_DEBUG_UART_BUFFER_RX);
    elib_queue_init(&queue_tx, buffer_tx, ELAB_DEBUG_UART_BUFFER_TX); 
}

/**
  * @brief  Send data to the debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
int16_t elab_debug_uart_send(void *buffer, uint16_t size)
{
    int16_t ret = 0;
    
    HAL_NVIC_DisableIRQ(DEBUG_USART_IRQ);
    
    if (elib_queue_is_empty(&queue_tx))
    {
        ret = elib_queue_push(&queue_tx, buffer, size);
        if (elib_queue_pull_pop(&queue_tx, &byte_send, 1) == 1)
        {
            HAL_UART_Transmit_IT(&huart, &byte_send, 1);//启动发送
        }
    }
    else
    {
        ret = elib_queue_push(&queue_tx, buffer, size);
    }
    
    HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ);

    return ret;
}

/**
  * @brief  Initialize the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
int16_t elab_debug_uart_receive(void *buffer, uint16_t size)
{
    int16_t ret = 0;

    HAL_NVIC_DisableIRQ(DEBUG_USART_IRQ);
    ret = elib_queue_pull_pop(&queue_rx, buffer, size);
    HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ);

    return ret;
}

/**
  * @brief  Clear buffer of the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
void elab_debug_uart_buffer_clear(void)
{
    HAL_NVIC_DisableIRQ(DEBUG_USART_IRQ);

    elib_queue_clear(&queue_rx);
    elib_queue_clear(&queue_tx);

    HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ);
}

void elab_debug_uart_txCallback(void)
{
    uint8_t byte = 0;
    if (elib_queue_pull_pop(&queue_tx, &byte, 1))
    {
        HAL_UART_Transmit_IT(&huart, &byte, 1);//继续启动发送
    }
}

void elab_debug_uart_rxCallback(void)
{
    elib_queue_push(&queue_rx, &byte_recv, 1);//接收到消息队列
    HAL_UART_Receive_IT(&huart, &byte_recv, 1);//继续启动接收  
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  The weak UART tx callback function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None.
  */
/*
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    uint8_t byte = 0;
    
    if (UartHandle->Instance == DEBUG_USART)
    {
        if (elib_queue_pull_pop(&queue_tx, &byte, 1))
        {
            HAL_UART_Transmit_IT(&huart, &byte, 1);//继续启动发送
        }
    }
}
*/
/**
  * @brief  The weak UART rx callback function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None.
  */
/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    if (UartHandle->Instance == DEBUG_USART)
    {
        elib_queue_push(&queue_rx, &byte_recv, 1);//接收到消息队列
        HAL_UART_Receive_IT(&huart, &byte_recv, 1);//继续启动接收  
    }
}
*/

/**
  * @brief  The weak UART initialization function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (uartHandle->Instance == DEBUG_USART)
    {
        /* USART clock enable */
         DEBUG_USART_CLK_ENABLE();
	
         DEBUG_USART_RX_GPIO_CLK_ENABLE();
         DEBUG_USART_TX_GPIO_CLK_ENABLE();

        /* 配置Tx引脚为复用功能  */
        GPIO_InitStruct.Pin = DEBUG_USART_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = DEBUG_USART_TX_AF;
        HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStruct);
        
        /* 配置Rx引脚为复用功能 */
        GPIO_InitStruct.Pin = DEBUG_USART_RX_PIN;
        GPIO_InitStruct.Alternate = DEBUG_USART_RX_AF;
        HAL_GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStruct); 
       
        /* USART interrupt Init */
        HAL_NVIC_SetPriority(DEBUG_USART_IRQ, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ);
    }
}

/**
  * @brief This function handles USART interrupts.
  */
void DEBUG_USART_IRQHandler(void)
{     
    HAL_UART_IRQHandler(&huart);//中断处理后操作完成
}
/* ----------------------------- end of file -------------------------------- */
