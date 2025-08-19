
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
    
    // ���ݴ���ͨ��
    me->dma_handle.Instance =src->dam_inst;
    // ѡ�� DMA ͨ����ͨ������������
    me->dma_handle.Init.Channel = src->dma_channel; 
    // ���ݴ��䷽��Ϊ���赽�洢��	
    me->dma_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;	
    // ����Ĵ���ֻ��һ������ַ���õ���
    me->dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;
    // �洢����ַ�̶�
    me->dma_handle.Init.MemInc = DMA_MINC_ENABLE; 
    // // �������ݴ�СΪ���֣��������ֽ� 
    me->dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; 
    //	�洢�����ݴ�СҲΪ���֣����������ݴ�С��ͬ
    me->dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;	
    // ѭ������ģʽ
    me->dma_handle.Init.Mode = DMA_CIRCULAR;
    // DMA ����ͨ�����ȼ�Ϊ�ߣ���ʹ��һ��DMAͨ��ʱ�����ȼ����ò�Ӱ��
    me->dma_handle.Init.Priority = DMA_PRIORITY_HIGH;
    // ��ֹDMA FIFO	��ʹ��ֱ��ģʽ
    me->dma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;  
    // FIFO ��С��FIFOģʽ��ֹʱ�������������	
    me->dma_handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
    me->dma_handle.Init.MemBurst = DMA_MBURST_SINGLE;
    me->dma_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;  
    //��ʼ��DMA�������൱��һ����Ĺܵ����ܵ������кܶ�ͨ��
    HAL_DMA_Init(&me->dma_handle); 
    
    __HAL_LINKDMA(&me->adc_handle, DMA_Handle, me->dma_handle);
    
    // -------------------ADC Init �ṹ�� ���� ��ʼ��------------------------
    me->adc_handle.Instance = src->adc_inst;
    // ʱ��Ϊfpclk 4��Ƶ	
    me->adc_handle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
    // ADC �ֱ���
    me->adc_handle.Init.Resolution = ADC_RESOLUTION_12B;
    // ��ֹɨ��ģʽ����ͨ���ɼ�����Ҫ	
    me->adc_handle.Init.ScanConvMode = DISABLE; 
    // ����ת��	
    me->adc_handle.Init.ContinuousConvMode = ENABLE;
    // ������ת��	
    me->adc_handle.Init.DiscontinuousConvMode = DISABLE;
    //��ֹ�ⲿ���ش���    
    me->adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    //�����Ҷ���	
    me->adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    //ת��ͨ�� 1��
    me->adc_handle.Init.NbrOfConversion = 1;
    //ʹ������ת������
    me->adc_handle.Init.DMAContinuousRequests = ENABLE;
    //ת����ɱ�־
    me->adc_handle.Init.EOCSelection          = DISABLE;    
    // ��ʼ��ADC	                          
    HAL_ADC_Init(&me->adc_handle);
    
    //---------------------------------------------------------------------------
    me->adc_conf.Channel      = get_adc_channel_from_num(channel_num);
    me->adc_conf.Rank         = 1;
    // ����ʱ����	
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
