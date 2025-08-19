
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "drv_adc.h"
#include "drv_util.h"
#include "elab/common/elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverADC");

#define ADC_DR_ADDR_OFFSET 0x4c

typedef struct adc_src
{
    const char *adc_name;
    ADC_TypeDef *adc_inst;
    const char *dma_name;
    DMA_Stream_TypeDef *dam_inst;
    uint32_t   dma_channel;
}adc_src_t;

/* private function prototype ----------------------------------------------- */
static uint32_t _get_value(elab_adc_t * const me);

/* private variables -------------------------------------------------------- */
static elab_adc_ops_t adc_driver_ops =
{
    .get_value = _get_value,
};

static adc_src_t adc_src[] = 
{
    {0},
    {"ADC1", ADC1, "DMA2", DMA2_Stream0, DMA_CHANNEL_0},
    {0},
    {0},
};

static void adc_gpio_config(elab_adc_driver_t *me, const char *pin_name);
static void adc_mode_config(elab_adc_driver_t *me, const char *adc_name, uint8_t channel_num);

/* public functions --------------------------------------------------------- */
void elab_driver_adc_init(elab_adc_driver_t *me, const char *name, 
                          const char *pin_name, const char *adc_name, uint8_t channel_num)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(pin_name != NULL);
    elab_assert(adc_name != NULL);

    //TODO: Innit adc
    adc_gpio_config(me, pin_name);
    adc_mode_config(me, adc_name, channel_num);
    
    elab_adc_register(&me->device, name, &adc_driver_ops, me);
}

static void adc_gpio_config(elab_adc_driver_t *me, const char *pin_name)
{
    elab_assert(me != NULL);
    elab_assert(pin_name != NULL);
    assert_name(check_pin_name_valid(pin_name), pin_name);

    gpio_clock_enable(pin_name);
    
    /* Configure GPIO pin. */
    GPIO_TypeDef *port = get_port_from_name(pin_name);
    uint16_t pin = get_pin_from_name(pin_name);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = pin;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

static void adc_mode_config(elab_adc_driver_t *me, const char *adc_name, uint8_t channel_num)
{
    elab_assert(me != NULL);
    elab_assert(adc_name != NULL);
    elab_assert(channel_num >= MIN_ADC_CHANNEL && channel_num <= MAX_ADC_CHANNEL);
    
    uint8_t num = get_adc_num_form_name(adc_name);
    elab_assert(num >= 1 && num <= 3);
    
    adc_src_t *src = (adc_src_t*)&adc_src[num];
    elab_assert(strcmp(adc_name, src->adc_name) == 0);
    
    adc_clock_enable(src->adc_name);
    dma_clock_enable(src->dma_name);
    
    //dma_irq_enable(src->dam_inst);
    
    me->dma_handle.Instance = src->dam_inst;
    me->dma_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    
    // 数据传输通道
    me->dma_handle.Instance =src->dam_inst;
    // 选择 DMA 通道，通道存在于流中
    me->dma_handle.Init.Channel = src->dma_channel; 
    // 数据传输方向为外设到存储器	
    me->dma_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;	
    // 外设寄存器只有一个，地址不用递增
    me->dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;
    // 存储器地址固定
    me->dma_handle.Init.MemInc = DMA_MINC_ENABLE; 
    // // 外设数据大小为半字，即两个字节 
    me->dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; 
    //	存储器数据大小也为半字，跟外设数据大小相同
    me->dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;	
    // 循环传输模式
    me->dma_handle.Init.Mode = DMA_CIRCULAR;
    // DMA 传输通道优先级为高，当使用一个DMA通道时，优先级设置不影响
    me->dma_handle.Init.Priority = DMA_PRIORITY_HIGH;
    // 禁止DMA FIFO	，使用直连模式
    me->dma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;  
    // FIFO 大小，FIFO模式禁止时，这个不用配置	
    me->dma_handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
    me->dma_handle.Init.MemBurst = DMA_MBURST_SINGLE;
    me->dma_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;  
    //初始化DMA流，流相当于一个大的管道，管道里面有很多通道
    HAL_DMA_Init(&me->dma_handle); 
    
    __HAL_LINKDMA(&me->adc_handle, DMA_Handle, me->dma_handle);
    
    // -------------------ADC Init 结构体 参数 初始化------------------------
    me->adc_handle.Instance = src->adc_inst;
    // 时钟为fpclk 4分频	
    me->adc_handle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
    // ADC 分辨率
    me->adc_handle.Init.Resolution = ADC_RESOLUTION_12B;
    // 禁止扫描模式，多通道采集才需要	
    me->adc_handle.Init.ScanConvMode = DISABLE; 
    // 连续转换	
    me->adc_handle.Init.ContinuousConvMode = ENABLE;
    // 非连续转换	
    me->adc_handle.Init.DiscontinuousConvMode = DISABLE;
    //禁止外部边沿触发    
    me->adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    //数据右对齐	
    me->adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    //转换通道 1个
    me->adc_handle.Init.NbrOfConversion = 1;
    //使能连续转换请求
    me->adc_handle.Init.DMAContinuousRequests = ENABLE;
    //转换完成标志
    me->adc_handle.Init.EOCSelection          = DISABLE;    
    // 初始化ADC	                          
    HAL_ADC_Init(&me->adc_handle);
    
    //---------------------------------------------------------------------------
    me->adc_conf.Channel      = get_adc_channel_from_num(channel_num);
    me->adc_conf.Rank         = 1;
    // 采样时间间隔	
    me->adc_conf.SamplingTime = ADC_SAMPLETIME_15CYCLES;//ADC_SAMPLETIME_56CYCLES;
  
    HAL_ADC_ConfigChannel(&me->adc_handle, &me->adc_conf);
    HAL_ADC_Start_DMA(&me->adc_handle, (uint32_t*)&me->adc_value, 1);
}

static uint32_t _get_value(elab_adc_t * const me)
{   
    elab_assert(me != NULL);
    elab_adc_driver_t *driver = (elab_adc_driver_t *)me->super.user_data;
    elab_assert(driver != NULL);
    
    return driver->adc_value;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
