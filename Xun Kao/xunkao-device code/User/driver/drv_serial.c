
/* Includes ------------------------------------------------------------------*/
#include "drv_serial.h"
#include "drv_util.h"
#include "../bsp/elab_uart_debug.h"
#include "elab/common/elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverSerial");

typedef struct uart_src
{
    char               *uart_name;
    USART_TypeDef      *uart_inst;
    char               *pin_name_tx;
    char               *pin_name_rx;
    
    char               *dma_name;
    DMA_Stream_TypeDef *dma_inst_tx;
    uint32_t           dma_channel_tx;
    DMA_Stream_TypeDef *dma_inst_rx;
    uint32_t           dma_channel_rx;
    
    
    
    elab_serial_t      *serial;
    UART_HandleTypeDef *huart;
    DMA_HandleTypeDef  *hdma_tx;
    DMA_HandleTypeDef  *hdma_rx;
    uint8_t             *buffer_rx;
    
}uart_src_t;

static void uart_src_init(elab_serial_driver_t *me, const elab_serial_config_t* config);

static elab_err_t uart_enable(elab_serial_t* const me, bool status);
static elab_err_t uart_write(elab_serial_t* const me,  const void* buffer, uint32_t size);
static void uart_set_tx(elab_serial_t* const me, bool status);
static elab_err_t uart_config(elab_serial_t * const me, elab_serial_config_t* const config);

static elab_serial_ops_t serial_driver_ops =
{
     .enable = uart_enable,
     .write = uart_write,
     .set_tx  = uart_set_tx, 
     .config = uart_config,
}; 

static uart_src_t uart_src[] = 
{
    {0},
    {"UART1", USART1, "A.09", "A.10", "DMA2", DMA2_Stream7, DMA_CHANNEL_4, DMA2_Stream2, DMA_CHANNEL_4},//for test!
    {"UART2", USART2, "D.05", "D.06", "DMA1", DMA1_Stream6, DMA_CHANNEL_4,DMA1_Stream5, DMA_CHANNEL_4},
    {0},
    {0},
    {0},
    {"UART6", USART6, "C.06", "C.07", "DMA2", DMA2_Stream6, DMA_CHANNEL_5, DMA2_Stream1, DMA_CHANNEL_5}
};

/* public function ---------------------------------------------------------- */

void elab_driver_uart_init(elab_serial_driver_t *me, const char *name, const elab_serial_config_t config)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    
    elab_assert_name((strcmp(name, "UART1") == 0
                      || strcmp(name, "UART2") == 0
                      || strcmp(name, "UART3") == 0
                      || strcmp(name, "UART4") == 0
                      || strcmp(name, "UART5") == 0
                      || strcmp(name, "UART6") == 0), name);
    me->name = name;
    memcpy(&me->config, &config, sizeof(elab_serial_config_t));
    
    elab_serial_attr_t attr = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    attr.baud_rate = config.baud_rate;
    attr.data_bits = config.data_bits;
    attr.mode = config.mode;
    attr.parity = config.parity;
    attr.stop_bits = config.stop_bits;
    elab_serial_register(&me->device, name, &serial_driver_ops, &attr, me);
    
    uart_src_init(me, &config);
}

static void uart_src_init(elab_serial_driver_t *me, const elab_serial_config_t* config)
{
    elab_assert(me != NULL);
    elab_assert(config != NULL);
    uint8_t num = get_uart_num_form_name(me->name);
    elab_assert(num >= 1 && num <= 6);
    
    uart_src_t *src = (uart_src_t*)&uart_src[num];
    elab_assert(strcmp(me->name, src->uart_name) == 0);
    
    //Save handler
    src->serial = &me->device;
    src->huart = &me->huart;
    src->hdma_tx = &me->hdma_uart_tx;
    src->hdma_rx = &me->hdma_uart_rx;
    src->buffer_rx = &me->buffer_rx;
    
    //Clock enable
    gpio_clock_enable(src->pin_name_tx);
    gpio_clock_enable(src->pin_name_rx);
    uart_clock_enable(src->uart_name);
    dma_clock_enable(src->dma_name);
    
    //Irq enabe
    uart_irq_enable(src->uart_inst);
    dma_irq_enable(src->dma_inst_tx);
    dma_irq_enable(src->dma_inst_rx);
    
    me->huart.Instance = src->uart_inst;
    me->huart.Init.BaudRate = config->baud_rate;
    me->huart.Init.WordLength = config->data_bits;
    me->huart.Init.StopBits = config->stop_bits;
    me->huart.Init.Parity = config->parity;
    me->huart.Init.Mode = UART_MODE_TX_RX;
    me->huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    me->huart.Init.OverSampling = UART_OVERSAMPLING_16;
    elab_assert(HAL_UART_Init(&me->huart) == HAL_OK);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_TypeDef *port_tx = get_port_from_name(src->pin_name_tx);
    GPIO_TypeDef *port_rx = get_port_from_name(src->pin_name_rx);
    uint16_t pin_tx = get_pin_from_name(src->pin_name_tx);
    uint16_t pin_rx = get_pin_from_name(src->pin_name_rx);
    uint32_t alter = get_altet_from_uart(src->uart_name);
    GPIO_InitStruct.Pin = pin_tx | pin_rx;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = alter;
    HAL_GPIO_Init(port_tx, &GPIO_InitStruct);
    
    me->hdma_uart_tx.Instance = src->dma_inst_tx;
    me->hdma_uart_tx.Init.Channel=src->dma_channel_tx;                 //通道选择
    me->hdma_uart_tx.Init.Direction=DMA_MEMORY_TO_PERIPH;             //存储器到外设
    me->hdma_uart_tx.Init.PeriphInc=DMA_PINC_DISABLE;                 //外设非增量模式
    me->hdma_uart_tx.Init.MemInc=DMA_MINC_ENABLE;                     //存储器增量模式
    me->hdma_uart_tx.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;    //外设数据长度:8位
    me->hdma_uart_tx.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;       //存储器数据长度:8位
    me->hdma_uart_tx.Init.Mode=DMA_NORMAL;                            //外设普通模式
    me->hdma_uart_tx.Init.Priority=DMA_PRIORITY_HIGH;               
    me->hdma_uart_tx.Init.FIFOMode=DMA_FIFOMODE_DISABLE;              //禁用FIFO
    //me->hdma_uart_tx.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;      
    //me->hdma_uart_tx.Init.MemBurst=DMA_MBURST_SINGLE;                 //存储器突发单次传输
    //me->hdma_uart_tx.Init.PeriphBurst=DMA_PBURST_SINGLE;              //外设突发单次传输
    elab_assert(HAL_DMA_Init(&me->hdma_uart_tx) == HAL_OK);

    __HAL_LINKDMA(&me->huart, hdmatx, me->hdma_uart_tx);
    
    me->hdma_uart_rx.Instance = src->dma_inst_rx;
    me->hdma_uart_rx.Init.Channel=src->dma_channel_rx;                 //通道选择
    me->hdma_uart_rx.Init.Direction=DMA_PERIPH_TO_MEMORY;             //外设到存储器
    me->hdma_uart_rx.Init.PeriphInc=DMA_PINC_DISABLE;                 //外设非增量模式
    me->hdma_uart_rx.Init.MemInc=DMA_MINC_ENABLE;                     //存储器增量模式
    me->hdma_uart_rx.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;    //外设数据长度:8位
    me->hdma_uart_rx.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;       //存储器数据长度:8位
    me->hdma_uart_rx.Init.Mode=DMA_NORMAL;                            //外设普通模式
    me->hdma_uart_rx.Init.Priority=DMA_PRIORITY_LOW;                
    me->hdma_uart_rx.Init.FIFOMode=DMA_FIFOMODE_DISABLE;              //禁用FIFO
    //me->hdma_uart_rx.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;      
    //me->hdma_uart_rx.Init.MemBurst=DMA_MBURST_SINGLE;                 //存储器突发单次传输
    //me->hdma_uart_rx.Init.PeriphBurst=DMA_PBURST_SINGLE;              //外设突发单次传输
    elab_assert(HAL_DMA_Init(&me->hdma_uart_rx) == HAL_OK);

    __HAL_LINKDMA(&me->huart, hdmarx, me->hdma_uart_rx);
    
    //HAL_UART_Receive_DMA(&me->huart, me->buffer_rx, BUFFER_RX_DEFAULT); //启动DMA接收
    HAL_UART_Receive_DMA(&me->huart, &me->buffer_rx, 1); //启动DMA接收
}

/**
  * @brief This function handles DMA interrupts.
  */
void DMA1_Stream6_IRQHandler(void)//UART2 TX
{  
    HAL_DMA_IRQHandler(uart_src[2].hdma_tx);
}

void DMA1_Stream5_IRQHandler(void)//UART2 RX
{    
    HAL_DMA_IRQHandler(uart_src[2].hdma_rx);
}

/**
  * @brief This function handles DMA interrupts.
  */
void DMA2_Stream6_IRQHandler(void)//UART6 TX
{  
    HAL_DMA_IRQHandler(uart_src[6].hdma_tx);
}

void DMA2_Stream1_IRQHandler(void)//UART6 RX
{    
    HAL_DMA_IRQHandler(uart_src[6].hdma_rx);
}

/**
  * @brief This function handles DMA interrupts.
  */
void DMA2_Stream7_IRQHandler(void)//UART1 TX
{  
    HAL_DMA_IRQHandler(uart_src[1].hdma_tx);
}

void DMA2_Stream2_IRQHandler(void)//UART1 RX
{    
    HAL_DMA_IRQHandler(uart_src[1].hdma_rx);
}

//void USART1_IRQHandler(void)
//{
//    HAL_UART_IRQHandler(uart_src[1].huart);
//}

void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(uart_src[2].huart);
}

void USART6_IRQHandler(void)
{
    HAL_UART_IRQHandler(uart_src[6].huart);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    if (UartHandle->Instance == DEBUG_USART)
    {
        elab_debug_uart_txCallback();
    }
    else
    {
        for(int32_t i = 0; i < sizeof(uart_src)/sizeof(uart_src[0]); i++)
        {
            if (uart_src[i].uart_inst == UartHandle->Instance)
            {
                elab_serial_tx_end(uart_src[i].serial);//elab_serial发送完成处理
            }
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    if (UartHandle->Instance == DEBUG_USART)
    {
        elab_debug_uart_rxCallback();
    }
    else
    {
        for(int32_t i = 0; i < sizeof(uart_src)/sizeof(uart_src[0]); i++)
        {
            if (uart_src[i].uart_inst == UartHandle->Instance)
            {
                elab_serial_isr_rx(uart_src[i].serial, uart_src[i].buffer_rx, 1);    
                HAL_UART_Receive_DMA(uart_src[i].huart, uart_src[i].buffer_rx, 1);//重启DMA接收
            }
        }
    }
}

/*_enable：无操作*/
static elab_err_t uart_enable(elab_serial_t *me, bool status)
{
    (void)me;
    (void)status;
    
    return ELAB_OK;
}

/*_write: 发送*/
static elab_err_t uart_write(elab_serial_t *me, const void *buffer, uint32_t size)
{
    elab_assert(me != NULL);
    elab_serial_driver_t *driver = (elab_serial_driver_t *)me->super.user_data;
    elab_assert(driver != NULL);
    UART_HandleTypeDef *huart = &driver->huart;
    elab_assert(huart != NULL);
    HAL_StatusTypeDef status = HAL_OK;
    
    status = HAL_UART_Transmit_DMA(huart, (uint8_t*)buffer, size);//启动DMA发送
    (void)status;
    
    return ELAB_OK;
}
/*_set_tx: uart默认为双线全双工模式，RS485半双工由上层软件通过使能引脚控制收发时序*/
static void uart_set_tx(elab_serial_t *me, bool status)
{
    (void)me;
    (void)status;
}
/*_config：无操作*/
static elab_err_t uart_config(elab_serial_t *me, elab_serial_config_t *config)
{
    (void)me;
    (void)config;
    return ELAB_OK;
}

#ifdef __cplusplus
}
#endif
