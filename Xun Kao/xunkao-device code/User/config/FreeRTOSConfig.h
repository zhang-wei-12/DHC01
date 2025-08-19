/* USER CODE BEGIN Header */
/*
 * FreeRTOS Kernel V10.3.1
 * Portion Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Portion Copyright (C) 2019 StMicroelectronics, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */
/* USER CODE END Header */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * These parameters and more are described within the 'configuration' section of the
 * FreeRTOS API documentation available on the FreeRTOS.org web site.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* Section where include file can be added */
/* USER CODE END Includes */

/* Ensure definitions are only used by the compiler, and not by the assembler. */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
  #include <stdint.h>
  extern uint32_t SystemCoreClock;
/* USER CODE BEGIN 0 */
  extern void configureTimerForRunTimeStats(void);
  extern unsigned long getRunTimeCounterValue(void);
/* USER CODE END 0 */
#endif
#ifndef CMSIS_device_header
#define CMSIS_device_header "stm32f4xx.h"
#endif /* CMSIS_device_header */

#define configENABLE_FPU                         0
#define configENABLE_MPU                         0

/* ��1��RTOSʹ����ռʽ����������0��RTOSʹ��Э��ʽ��������ʱ��Ƭ��
 * 
 * ע���ڶ������������ϣ�����ϵͳ���Է�Ϊ��ռʽ��Э��ʽ���֡�
 * Э��ʽ����ϵͳ�����������ͷ�CPU���л�����һ������
 * �����л���ʱ����ȫȡ�����������е�����
 */
#define configUSE_PREEMPTION                     1
//1ʹ��ʱ��Ƭ����(Ĭ��ʽʹ�ܵ�)
#define configUSE_TIME_SLICING					 1	
/* ĳЩ����FreeRTOS��Ӳ�������ַ���ѡ����һ��Ҫִ�е�����
 * ͨ�÷������ض���Ӳ���ķ��������¼�ơ����ⷽ��������
 * 
 * ͨ�÷�����
 *      1.configUSE_PORT_OPTIMISED_TASK_SELECTION Ϊ 0 ����Ӳ����֧���������ⷽ����
 *      2.������������FreeRTOS֧�ֵ�Ӳ��
 *      3.��ȫ��Cʵ�֣�Ч���Ե������ⷽ����
 *      4.��ǿ��Ҫ���������������ȼ���Ŀ
 * ���ⷽ����
 *      1.���뽫configUSE_PORT_OPTIMISED_TASK_SELECTION����Ϊ1��
 *      2.����һ�������ض��ܹ��Ļ��ָ�һ�������Ƽ���ǰ����[CLZ]ָ���
 *      3.��ͨ�÷�������Ч
 *      4.һ��ǿ���޶����������ȼ���ĿΪ32
 * һ����Ӳ������ǰ����ָ������ʹ�õģ�MCUû����ЩӲ��ָ��Ļ��˺�Ӧ������Ϊ0��
 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0
/* ��1��ʹ�ܵ͹���ticklessģʽ����0������ϵͳ���ģ�tick���ж�һֱ����
 * ���迪���͹��ĵĻ����ܻᵼ�����س������⣬��Ϊ������˯����,�������°취���
 * 
 * ���ط�����
 *      1.���������������Ӻ�
 *      2.��ס��λ�������������˲���ɿ���λ����
 *     
 *      1.ͨ������ñ�� BOOT 0 �Ӹߵ�ƽ(3.3V)
 *      2.�����ϵ磬����
 *    
 * 			1.ʹ��FlyMcu����һ��оƬ��Ȼ���������
 *			STMISP -> ���оƬ(z)
 */
#define configUSE_TICKLESS_IDLE													0  
/*
 * д��ʵ�ʵ�CPU�ں�ʱ��Ƶ�ʣ�Ҳ����CPUָ��ִ��Ƶ�ʣ�ͨ����ΪFclk
 * FclkΪ����CPU�ں˵�ʱ���źţ�������˵��cpu��ƵΪ XX MHz��
 * ����ָ�����ʱ���źţ���Ӧ�ģ�1/Fclk��Ϊcpuʱ�����ڣ�
 */
#define configCPU_CLOCK_HZ                       ( SystemCoreClock )
//RTOSϵͳ�����жϵ�Ƶ�ʡ���һ���жϵĴ�����ÿ���ж�RTOS��������������
#define configTICK_RATE_HZ                       ((TickType_t)1000)
//��ʹ�õ�������ȼ�
#define configMAX_PRIORITIES                     ( 56 )
//��������ʹ�õĶ�ջ��С
#define configMINIMAL_STACK_SIZE                 ((uint16_t)(512 / 4))
//���������ַ�������
#define configMAX_TASK_NAME_LEN                  ( 16 )
 //ϵͳ���ļ����������������ͣ�1��ʾΪ16λ�޷������Σ�0��ʾΪ32λ�޷�������
#define configUSE_16_BIT_TICKS                   0
//�����������CPUʹ��Ȩ������ͬ���ȼ����û�����
#define configIDLE_SHOULD_YIELD					1     

//ʹ�û����ź���
#define configUSE_MUTEXES                        1
//ʹ�õݹ黥���ź���      
#define configUSE_RECURSIVE_MUTEXES              1
//Ϊ1ʱʹ�ü����ź���
#define configUSE_COUNTING_SEMAPHORES            1
/* ���ÿ���ע����ź�������Ϣ���и��� */
#define configQUEUE_REGISTRY_SIZE                10

/*****************************************************************
              FreeRTOS���ڴ������й�����ѡ��                                               
*****************************************************************/
//֧�ֶ�̬�ڴ�����
#define configSUPPORT_STATIC_ALLOCATION          1
//֧�־�̬�ڴ�
#define configSUPPORT_DYNAMIC_ALLOCATION         1
//ϵͳ�����ܵĶѴ�С
#define configTOTAL_HEAP_SIZE                    ((size_t)44 * 1024)//((size_t)36 * 1024)


/***************************************************************
             FreeRTOS�빳�Ӻ����йص�����ѡ��                                            
**************************************************************/
/* ��1��ʹ�ÿ��й��ӣ�Idle Hook�����ڻص�����������0�����Կ��й���
 * 
 * ������������һ������������������û���ʵ�֣�
 * FreeRTOS�涨�˺��������ֺͲ�����void vApplicationIdleHook(void )��
 * ���������ÿ�������������ڶ��ᱻ����
 * �����Ѿ�ɾ����RTOS���񣬿�����������ͷŷ�������ǵĶ�ջ�ڴ档
 * ��˱��뱣֤����������Ա�CPUִ��
 * ʹ�ÿ��й��Ӻ�������CPU����ʡ��ģʽ�Ǻܳ�����
 * �����Ե��û������������������API����
 */
#define configUSE_IDLE_HOOK                      0
/* ��1��ʹ��ʱ��Ƭ���ӣ�Tick Hook������0������ʱ��Ƭ����
 * 
 * 
 * ʱ��Ƭ������һ������������������û���ʵ�֣�
 * FreeRTOS�涨�˺��������ֺͲ�����void vApplicationTickHook(void )
 * ʱ��Ƭ�жϿ��������Եĵ���
 * ��������ǳ���С�����ܴ���ʹ�ö�ջ��
 * ���ܵ����ԡ�FromISR" �� "FROM_ISR����β��API����
 */
 /*xTaskIncrementTick��������xPortSysTickHandler�жϺ����б����õġ���ˣ�vApplicationTickHook()����ִ�е�ʱ�����̲ܶ���*/
#define configUSE_TICK_HOOK                      0
//ʹ���ڴ�����ʧ�ܹ��Ӻ���
#define configUSE_MALLOC_FAILED_HOOK			0 
/*
 * ����0ʱ���ö�ջ�����⹦�ܣ����ʹ�ô˹��� 
 * �û������ṩһ��ջ������Ӻ��������ʹ�õĻ�
 * ��ֵ����Ϊ1����2����Ϊ������ջ�����ⷽ�� */
#define configCHECK_FOR_STACK_OVERFLOW           2

/********************************************************************
          FreeRTOS������ʱ�������״̬�ռ��йص�����ѡ��   
**********************************************************************/
//��������ʱ��ͳ�ƹ���
#define configGENERATE_RUN_TIME_STATS            0
 //���ÿ��ӻ����ٵ���
#define configUSE_TRACE_FACILITY                 1
 /* ���configUSE_TRACE_FACILITYͬʱΪ1ʱ���������3������
 * prvWriteNameToBuffer()
 * vTaskList(),
 * vTaskGetRunTimeStats()
*/
#define configUSE_STATS_FORMATTING_FUNCTIONS     1


/* USER CODE BEGIN MESSAGE_BUFFER_LENGTH_TYPE */
/* Defaults to size_t for backward compatibility, but can be changed
   if lengths will always be less than the number of bytes in a size_t. */
#define configMESSAGE_BUFFER_LENGTH_TYPE         size_t
/* USER CODE END MESSAGE_BUFFER_LENGTH_TYPE */

/********************************************************************
                FreeRTOS��Э���йص�����ѡ��                                                
*********************************************************************/
/* Co-routine definitions. */
//����Э�̣�����Э���Ժ��������ļ�croutine.c
#define configUSE_CO_ROUTINES                    0
//Э�̵���Ч���ȼ���Ŀ
#define configMAX_CO_ROUTINE_PRIORITIES          ( 2 )

/***********************************************************************
                FreeRTOS�������ʱ���йص�����ѡ��      
**********************************************************************/
/* Software timer definitions. */
 //���������ʱ��
#define configUSE_TIMERS                         1
//�����ʱ�����ȼ�
#define configTIMER_TASK_PRIORITY                ( 40 )
//�����ʱ�����г���
#define configTIMER_QUEUE_LENGTH                 10
//�����ʱ�������ջ��С
#define configTIMER_TASK_STACK_DEPTH             (1024 / 4)

/* The following flag must be enabled only when using newlib */
#define configUSE_NEWLIB_REENTRANT          0

/* CMSIS-RTOS V2 flags */
#define configUSE_OS2_THREAD_SUSPEND_RESUME  1
#define configUSE_OS2_THREAD_ENUMERATE       1
#define configUSE_OS2_EVENTFLAGS_FROM_ISR    1
#define configUSE_OS2_THREAD_FLAGS           1
#define configUSE_OS2_TIMER                  1
#define configUSE_OS2_MUTEX                  1
/*
 * The CMSIS-RTOS V2 FreeRTOS wrapper is dependent on the heap implementation used
 * by the application thus the correct define need to be enabled below
 */
#define USE_FreeRTOS_HEAP_4

/************************************************************
            FreeRTOS��ѡ��������ѡ��                                                     
************************************************************/
/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet             1
#define INCLUDE_uxTaskPriorityGet            1
#define INCLUDE_vTaskDelete                  1
#define INCLUDE_vTaskCleanUpResources        0
#define INCLUDE_vTaskSuspend                 1
#define INCLUDE_vTaskDelayUntil              1
#define INCLUDE_vTaskDelay                   1
#define INCLUDE_xTaskGetSchedulerState       1
#define INCLUDE_xTimerPendFunctionCall       1
#define INCLUDE_xQueueGetMutexHolder         1
#define INCLUDE_uxTaskGetStackHighWaterMark  1
#define INCLUDE_xTaskGetCurrentTaskHandle    1
#define INCLUDE_eTaskGetState                1

/******************************************************************
            FreeRTOS���ж��йص�����ѡ��                                                 
******************************************************************/
/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
 /* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
 #define configPRIO_BITS         __NVIC_PRIO_BITS
#else
 #define configPRIO_BITS         4
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
//�ж�������ȼ�
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
//ϵͳ�ɹ��������ж����ȼ�
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
/* USER CODE BEGIN 1 */
#define configASSERT( x ) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );}
/* USER CODE END 1 */


/****************************************************************
            FreeRTOS���жϷ������йص�����ѡ��                         
****************************************************************/
/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler    SVC_Handler
#define xPortPendSVHandler PendSV_Handler

/* IMPORTANT: After 10.3.1 update, Systick_Handler comes from NVIC (if SYS timebase = systick), otherwise from cmsis_os2.c */

#define USE_CUSTOM_SYSTICK_HANDLER_IMPLEMENTATION 1

/* USER CODE BEGIN 2 */
/* Definitions needed when configGENERATE_RUN_TIME_STATS is on */
//#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS vConfigureTimerForRunTimeStats
//#define portGET_RUN_TIME_COUNTER_VALUE getRunTimeCounterValue
/* USER CODE END 2 */

/* USER CODE BEGIN Defines */
/* Section where parameter definitions can be added (for instance, to override default ones in FreeRTOS.h) */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS	2
/* USER CODE END Defines */

#endif /* FREERTOS_CONFIG_H */
