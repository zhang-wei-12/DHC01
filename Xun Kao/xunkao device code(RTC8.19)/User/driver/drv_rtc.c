
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "drv_rtc.h"
#include "elab/common/elab_assert.h"
#include "stm32f4xx_hal_rcc.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverRTC");

/* private function prototype ----------------------------------------------- */
static elab_err_t _get_time(elab_rtc_t *me, elab_rtc_time_t *rtc_time);
static elab_err_t _set_time(elab_rtc_t *me, const elab_rtc_time_t *rtc_time);

/* private variables -------------------------------------------------------- */
static elab_rtc_ops_t rtc_driver_ops =
{
    .get_time = _get_time,
    .set_time = _set_time,
};

/* public functions --------------------------------------------------------- */
/*void elab_driver_rtc_init(elab_rtc_driver_t *me, const char *name)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    RTC_HandleTypeDef *hrtc = &me->hrtc;
    elab_assert(hrtc != NULL);
    
    hrtc->Instance = RTC;
    hrtc->Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc->Init.AsynchPrediv = 127;
    hrtc->Init.SynchPrediv = 255;
    hrtc->Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN; 
    
    HAL_RTC_Init(hrtc);
    //elab_assert(HAL_RTC_Init(hrtc) == HAL_OK);
    
    elab_rtc_register(&me->device, name, &rtc_driver_ops, me);
}*/
void elab_driver_rtc_init(elab_rtc_driver_t *me, const char *name) {
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    
    // ????????
    memset(&me->device, 0, sizeof(me->device));

    RTC_HandleTypeDef *hrtc = &me->hrtc;
    hrtc->Instance = RTC;
    hrtc->Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc->Init.AsynchPrediv = 127;
    hrtc->Init.SynchPrediv = 255;
    hrtc->Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    // 1. ?????(????????)
    __HAL_RCC_BACKUPRESET_FORCE();
    __HAL_RCC_BACKUPRESET_RELEASE();

    // 2. ?????
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTC_ENABLE();

    // 3. ???????
    if ((RCC->BDCR & RCC_BDCR_RTCSEL_Msk) == 0) {
        elog_warn("No RTC clock source selected, forcing LSI");
        
        // ???RTC?????
        CLEAR_BIT(RCC->BDCR, RCC_BDCR_RTCEN);
        
        __HAL_RCC_LSI_ENABLE();
        while (!__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY));
        
        MODIFY_REG(RCC->BDCR, RCC_BDCR_RTCSEL_Msk, RCC_RTCCLKSOURCE_LSI);
        SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN);
    }

    // 4. ????????
    hrtc->Instance->ISR = 0;
    hrtc->Instance->CR = 0;

    // 5. ???RTC
    HAL_StatusTypeDef status = HAL_RTC_Init(hrtc);
    if (status != HAL_OK) {
        elog_error("RTC init failed (status=%d). Registers:", status);
        elog_error("RCC_BDCR: 0x%08X", RCC->BDCR);
        elog_error("RTC_ISR: 0x%08X", hrtc->Instance->ISR);
        elog_error("RTC_PRER: 0x%08X", hrtc->Instance->PRER);
        return;
    }

    // 6. ???????
    uint32_t clocksource = __HAL_RCC_GET_RTC_SOURCE();
    elog_info("RTC clock source: %s", 
        (clocksource == RCC_RTCCLKSOURCE_LSE) ? "LSE" :
        (clocksource == RCC_RTCCLKSOURCE_LSI) ? "LSI" : "NONE");

    // 7. ????
    elab_rtc_register(&me->device, name, &rtc_driver_ops, me);
		HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR0, 0xA5A5); // ?????
		uint32_t readback = HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR0);
		if(readback != 0xA5A5) {
    elog_error("RTC backup register test failed!");
		}
    return; // ?????
}

static elab_err_t _get_time(elab_rtc_t *me, elab_rtc_time_t *rtc_time)
{
    elab_assert(me != NULL);
    elab_assert(rtc_time != NULL);
    
    elab_rtc_driver_t *driver = (elab_rtc_driver_t *)me->super.user_data;
    elab_assert(driver != NULL);
    RTC_HandleTypeDef *hrtc = &driver->hrtc;
    
    RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;
	
    elab_assert(HAL_RTC_GetTime(hrtc, &RTC_TimeStructure, RTC_FORMAT_BIN) == HAL_OK);
    elab_assert(HAL_RTC_GetDate(hrtc, &RTC_DateStructure, RTC_FORMAT_BIN) == HAL_OK);
    
    rtc_time->time.hour = RTC_TimeStructure.Hours;
    rtc_time->time.minute = RTC_TimeStructure.Minutes;
    rtc_time->time.second = RTC_TimeStructure.Seconds;
    
    rtc_time->date.year = RTC_DateStructure.Year + 2000;
    rtc_time->date.month = RTC_DateStructure.Month;
    rtc_time->date.day = RTC_DateStructure.Date;
    rtc_time->date.week = RTC_DateStructure.WeekDay;
    
    return ELAB_OK;
}

static elab_err_t _set_time(elab_rtc_t *me, const elab_rtc_time_t *rtc_time)
{
    elab_assert(me != NULL);
    elab_assert(rtc_time != NULL);
    
    elab_rtc_driver_t *driver = (elab_rtc_driver_t *)me->super.user_data;
    elab_assert(driver != NULL);
    RTC_HandleTypeDef *hrtc = &driver->hrtc;
    
    RTC_DateTypeDef  RTC_DateStructure;
    RTC_TimeTypeDef  RTC_TimeStructure;
	// 初始化时间
	RTC_TimeStructure.TimeFormat = RTC_HOURFORMAT_24;
	RTC_TimeStructure.Hours = rtc_time->time.hour;        
	RTC_TimeStructure.Minutes = rtc_time->time.minute;      
	RTC_TimeStructure.Seconds = rtc_time->time.second; 
    RTC_TimeStructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    RTC_TimeStructure.StoreOperation = RTC_STOREOPERATION_RESET;
    
    HAL_RTC_SetTime(hrtc,&RTC_TimeStructure, RTC_FORMAT_BIN);
    // 初始化日期	   
	RTC_DateStructure.Date = rtc_time->date.day;         
	RTC_DateStructure.Month = rtc_time->date.month;         
	RTC_DateStructure.Year = rtc_time->date.year - 2000;
    RTC_DateStructure.WeekDay = rtc_time->date.week;
    
    HAL_RTC_SetDate(hrtc, &RTC_DateStructure, RTC_FORMAT_BIN);
    //elab_assert(HAL_RTC_SetDate(hrtc, &RTC_DateStructure, RTC_FORMAT_BIN) == HAL_OK);
    
    return ELAB_OK;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
