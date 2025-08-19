/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include "drv_util.h"
#include "elab/common/elab_assert.h"
#include "../config/FreeRTOSConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverUtil");

/* public functions --------------------------------------------------------- */
bool check_pin_name_valid(const char *name)
{
    bool valid = true;

    if ((strlen(name) != 4) || name[1] != '.')
    {
        valid = false;
    }
    else if (!(name[0] >= 'A' && name[0] <= 'I'))
    {
        valid = false;
    }
    else if (!(name[2] >= '0' && name[2] <= '9'))
    {
        valid = false;
    }
    else if (!(name[3] >= '0' && name[3] <= '9'))
    {
        valid = false;
    }

    char *str_num = (char *)&name[2];
    int32_t pin_num = atoi(str_num);
    if (pin_num < 0 || pin_num >= 16)
    {
        valid = false;
    }

    return valid;
}

GPIO_TypeDef *get_port_from_name(const char *name)
{
    static const GPIO_TypeDef *port_table[] =
    {
        GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI
    };

    return (GPIO_TypeDef *)port_table[name[0] - 'A'];
}

uint16_t get_pin_from_name(const char *name)
{
    char *str_num = (char *)&name[2];
    int32_t pin_num = atoi(str_num);

    return (uint16_t)(1 << pin_num);
}

void gpio_clock_enable(const char *name)
{
    /* Enable the clock. */
    if (get_port_from_name(name) == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOD)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOE)
    {
        __HAL_RCC_GPIOE_CLK_DISABLE();
    }
    else if (get_port_from_name(name) == GPIOF)
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOG)
    {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOH)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOI)
    {
        __HAL_RCC_GPIOI_CLK_ENABLE();
    }
}

uint8_t get_uart_num_form_name(const char *name)
{
   char *str_num = (char*)&name[4];//UART1
   int32_t adc_num = atoi(str_num);
    
   return (uint8_t)adc_num;
}

uint32_t get_altet_from_uart(const char *name)
{
    uint8_t num = get_uart_num_form_name(name);
    uint32_t alter = 0;
    
    switch (num)
    {
        case 1:
            alter = GPIO_AF7_USART1;
            break;
        case 2:
            alter = GPIO_AF7_USART2;
            break;
        case 3:
            alter = GPIO_AF7_USART3;
            break;
        case 4:
            alter = GPIO_AF8_UART4;
            break;
        case 5:
            alter = GPIO_AF8_UART5;
            break;
        case 6:
            alter = GPIO_AF8_USART6;
            break;
        default:
            break;
    }
    
    return alter;
}

void uart_clock_enable(const char *name)
{   
    if (1 == get_uart_num_form_name(name))
    {
        __USART1_CLK_ENABLE();
    }
    else if (2 == get_uart_num_form_name(name))
    {
        __USART2_CLK_ENABLE();
    }
    else if (3 == get_uart_num_form_name(name))
    {
        __USART3_CLK_ENABLE();
    }
    else if (4 == get_uart_num_form_name(name))
    {
        __USART4_CLK_ENABLE();
    }
    else if (5 == get_uart_num_form_name(name))
    {
         __USART5_CLK_ENABLE();
    }
    else if (6 == get_uart_num_form_name(name))
    {
        __USART6_CLK_ENABLE();
    }
}

void dma_clock_enable(const char *name)
{
    if (strcmp(name, "DMA1") == 0)
    {
        __DMA1_CLK_ENABLE();
    }
    else if (strcmp(name, "DMA2") == 0)
    {
        __DMA2_CLK_ENABLE();
    }
}

void dma_irq_enable(DMA_Stream_TypeDef *inst)
{
    static struct dma_stream_irq
    {
        DMA_Stream_TypeDef *stream;
        IRQn_Type           irq;
    }dma_stream_irq_tbl[] = 
    {
        {DMA1_Stream0, DMA1_Stream0_IRQn},
        {DMA1_Stream1, DMA1_Stream1_IRQn},
        {DMA1_Stream2, DMA1_Stream2_IRQn},
        {DMA1_Stream3, DMA1_Stream3_IRQn},
        {DMA1_Stream4, DMA1_Stream4_IRQn},
        {DMA1_Stream5, DMA1_Stream5_IRQn},
        {DMA1_Stream6, DMA1_Stream6_IRQn},
        {DMA1_Stream7, DMA1_Stream7_IRQn},
        {DMA2_Stream0, DMA2_Stream0_IRQn},
        {DMA2_Stream1, DMA2_Stream1_IRQn},
        {DMA2_Stream2, DMA2_Stream2_IRQn},
        {DMA2_Stream3, DMA2_Stream3_IRQn},
        {DMA2_Stream4, DMA2_Stream4_IRQn},
        {DMA2_Stream5, DMA2_Stream5_IRQn},
        {DMA2_Stream6, DMA2_Stream6_IRQn},
        {DMA2_Stream7, DMA2_Stream7_IRQn},
    };
    
    for(int32_t i = 0; i < sizeof(dma_stream_irq_tbl)/sizeof(dma_stream_irq_tbl[0]); i++)
    {
        if (inst == dma_stream_irq_tbl[i].stream)
        {
            HAL_NVIC_SetPriority(dma_stream_irq_tbl[i].irq, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(dma_stream_irq_tbl[i].irq);
            break;
        }
    }
}

void uart_irq_enable(USART_TypeDef *inst)
{
    IRQn_Type irq;
    
    if(inst == USART1)
    {
        irq = USART1_IRQn;
    }
    else if (inst == USART2)
    {
        irq = USART2_IRQn;
    }
    else if (inst == USART3)
    {
        irq = USART3_IRQn;
    }
    else if (inst == UART4)
    {
        irq = UART4_IRQn;
    }
    else if (inst == UART5)
    {
        irq = UART5_IRQn;
    }
    else if (inst == USART6)
    {
        irq = USART6_IRQn;
    }
    
    HAL_NVIC_SetPriority(irq, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(irq);
}

uint8_t get_adc_num_form_name(const char *name)
{
   char *str_num = (char*)&name[3]; //ADC1
   int32_t uart_num = atoi(str_num);
    
   return (uint8_t)uart_num;
}

void adc_clock_enable(const char *name)
{   
    if (1 == get_adc_num_form_name(name))
    {
        __ADC1_CLK_ENABLE();
    }
    else if (2 == get_adc_num_form_name(name))
    {
        __ADC2_CLK_ENABLE();
    }
    else if (3 == get_adc_num_form_name(name))
    {
        __ADC3_CLK_ENABLE();
    }
}

uint32_t get_adc_channel_from_num(uint8_t channel_num)
{
    static uint32_t channel[] = {ADC_CHANNEL_0,ADC_CHANNEL_1,ADC_CHANNEL_2,ADC_CHANNEL_3,ADC_CHANNEL_4,
    ADC_CHANNEL_5,ADC_CHANNEL_6,ADC_CHANNEL_7,ADC_CHANNEL_8,ADC_CHANNEL_9,ADC_CHANNEL_10,ADC_CHANNEL_11,
    ADC_CHANNEL_12,ADC_CHANNEL_13,ADC_CHANNEL_14,ADC_CHANNEL_15,ADC_CHANNEL_16,ADC_CHANNEL_17,ADC_CHANNEL_18};
    
    elab_assert(channel_num < sizeof(channel)/sizeof(channel[0]));

    return channel[channel_num];
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
