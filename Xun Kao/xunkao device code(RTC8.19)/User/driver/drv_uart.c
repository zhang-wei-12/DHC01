/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "elab/common/elab_export.h"
#include "elab/common/elab_common.h"
#include "elab/common/elab_assert.h"
#include "elab/edf/normal/elab_serial.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverSpiBus");

/* private function prototypes ---------------------------------------------- */
static elab_err_t uart_enable(elab_serial_t *me, bool status);
static elab_err_t uart_read(elab_serial_t *me, void *buffer, uint32_t size);
static elab_err_t uart_write(elab_serial_t *me, const void *buffer, uint32_t size);
static elab_err_t uart_config(elab_serial_t *me, elab_serial_config_t *config);

/* private variables -------------------------------------------------------- */
static elab_serial_uart_t serail_uart2;
static UART_HandleTypeDef huart2;
static DMA_HandleTypeDef hdma_uart2_tx;
static DMA_HandleTypeDef hdma_uart2_rx;
static DMA_HandleTypeDef hdma_uart6_tx;
static DMA_HandleTypeDef hdma_uart6_rx;

static elab_serial_ops_t serial_uart_ops =
{
     .enable = uart_enable,
     .read = uart_read,
     .write = uart_write,
     .config = uart_config,
};

static elab_serial_attr_t serial_attr = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;

/* public function ---------------------------------------------------------- */
/* Uart2 init function */
static void uart2_export(void)
{
    __GPIOD_CLK_ENABLE();
    __USART2_CLK_ENABLE();
    __DMA1_CLK_ENABLE();

    /* DMA1_Stream5_IRQn DMA1_Stream6_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
    
    /*Init serial_attr*/
    serial_attr.mode = ELAB_SERIAL_MODE_HALF_DUPLEX;

    huart2.Instance = USART2;
    huart2.Init.BaudRate = serial_attr.baud_rate;
    huart2.Init.WordLength = serial_attr.data_bits;
    huart2.Init.StopBits = serial_attr.stop_bits;
    huart2.Init.Parity = serial_attr.parity;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (serial_attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX)
        HAL_HalfDuplex_Init(&huart2); //半双工
    else
        HAL_UART_Init(&huart2);
    
    /**USART2 GPIO Configuration    
    PD5    ------> USART2_TX
    PD6    ------> USART2_RX 
    */
    /* 配置Tx引脚为复用功能  */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
    /* 配置Rx引脚为复用功能 */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct); 

    /* Uart DMA Init */
    /* Uart2_TX Init */
    hdma_uart2_tx.Instance = DMA1_Stream6;
    hdma_uart2_tx.Init.Channel=DMA_CHANNEL_4;                       //通道选择
    hdma_uart2_tx.Init.Direction=DMA_MEMORY_TO_PERIPH;             //存储器到外设
    hdma_uart2_tx.Init.PeriphInc=DMA_PINC_DISABLE;                 //外设非增量模式
    hdma_uart2_tx.Init.MemInc=DMA_MINC_ENABLE;                     //存储器增量模式
    hdma_uart2_tx.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;    //外设数据长度:8位
    hdma_uart2_tx.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;       //存储器数据长度:8位
    hdma_uart2_tx.Init.Mode=DMA_NORMAL;                            //外设普通模式
    hdma_uart2_tx.Init.Priority=DMA_PRIORITY_LOW;                  //低等优先级
    hdma_uart2_tx.Init.FIFOMode=DMA_FIFOMODE_DISABLE;              //禁用FIFO
    hdma_uart2_tx.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;      
    hdma_uart2_tx.Init.MemBurst=DMA_MBURST_SINGLE;                 //存储器突发单次传输
    hdma_uart2_tx.Init.PeriphBurst=DMA_PBURST_SINGLE;              //外设突发单次传输
    HAL_DMA_Init(&hdma_uart2_tx);

    __HAL_LINKDMA(&huart2, hdmatx, hdma_uart2_tx);

    /* Uart2_RX Init */
    hdma_uart2_rx.Instance = DMA1_Stream5;
    hdma_uart2_tx.Init.Channel=DMA_CHANNEL_4;                       //通道选择
    hdma_uart2_tx.Init.Direction=DMA_MEMORY_TO_PERIPH;             //存储器到外设
    hdma_uart2_tx.Init.PeriphInc=DMA_PINC_DISABLE;                 //外设非增量模式
    hdma_uart2_tx.Init.MemInc=DMA_MINC_ENABLE;                     //存储器增量模式
    hdma_uart2_tx.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;    //外设数据长度:8位
    hdma_uart2_tx.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;       //存储器数据长度:8位
    hdma_uart2_tx.Init.Mode=DMA_NORMAL;                            //外设普通模式
    hdma_uart2_tx.Init.Priority=DMA_PRIORITY_LOW;                  //低等优先级
    hdma_uart2_tx.Init.FIFOMode=DMA_FIFOMODE_DISABLE;              //禁用FIFO
    hdma_uart2_tx.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;      
    hdma_uart2_tx.Init.MemBurst=DMA_MBURST_SINGLE;                 //存储器突发单次传输
    hdma_uart2_tx.Init.PeriphBurst=DMA_PBURST_SINGLE;              //外设突发单次传输
    HAL_DMA_Init(&hdma_uart2_rx);

    __HAL_LINKDMA(&huart2, hdmarx, hdma_uart2_tx);

    elab_serial_uart_register(&serail_uart2, "UART2", &serial_uart_ops, &serial_attr, &huart2);
}
INIT_EXPORT(uart2_export, EXPORT_LEVEL_BSP);

/* private functions -------------------------------------------------------- */
/*_enable: 无任何操作*/
static elab_err_t uart_enable(elab_serial_t *const me, bool status)
{
    (void)me;
    (void)status;
    
    return ELAB_OK;
}
/*_read: 接收*/
static elab_err_t uart_read(elab_serial_t *const me, void *buffer, uint32_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)me->uart->super.user_data;
    
    status = HAL_UART_Receive_DMA(huart, (uint8_t*)buffer, size);
    (void)status;
    
    return ELAB_OK;
}
/*_write: 发送*/
static elab_err_t uart_write(elab_serial_t *const me, const void *buffer, uint32_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)me->uart->super.user_data;
    
    status = HAL_UART_Transmit_DMA(huart, (uint8_t*)buffer, size);
    (void)status;
    
    return ELAB_OK;
}

/*_config驱动方法*/
static elab_err_t uart_config(elab_serial_t *me, elab_serial_config_t *config)
{
    (void)me;
    (void)config;
    return ELAB_OK;
}

/* private functions -------------------------------------------------------- */
/**
  * @brief This function handles DMA interrupts.
  */
void DMA1_Stream5_IRQHandler(void)//UART2 RX
{    
    HAL_DMA_IRQHandler(&hdma_uart2_rx);
}

void DMA1_Stream6_IRQHandler(void)//UART2 TX
{  
    HAL_DMA_IRQHandler(&hdma_uart2_tx);
}

/*DMA传输完成回调方法*/
void XferCpltCallback ( struct __DMA_HandleTypeDef * hdma)
{
    //Todo：将上次接收完成的数据传入elab_serial_isr_rx
    if (hdma->Instance == DMA1_Stream5)//UART2 RX
    {
        elab_serial_uart_rx_end(&serail_uart2);//elab_serial接收完成处理
    }
    else if (hdma->Instance == DMA1_Stream6)//UART2 TX
    {
        elab_serial_uart_tx_end(&serail_uart2);//elab_serial发送完成处理
    }
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
