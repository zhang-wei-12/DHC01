
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "drv_watchdog.h"
#include "elab/common/elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverWatchdog");

/* private function prototype ----------------------------------------------- */
static void _set_time(elab_watchdog_t *me, uint32_t timeout_ms);
static void _feed(elab_watchdog_t *me);

/* private variables -------------------------------------------------------- */
static elab_watchdog_ops_t watchdog_driver_ops =
{
    .set_time = _set_time,
    .feed = _feed,
};

/* public functions --------------------------------------------------------- */
void elab_driver_watchdog_init(elab_watchdog_driver_t *me, const char *name)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    
    IWDG_HandleTypeDef *hiwdg = &me->hiwdg;
    
    hiwdg->Instance = IWDG; // ѡ��������Ź�IWDG
    // ���ÿ��Ź�
    hiwdg->Init.Prescaler = IWDG_PRESCALER_64; // Ԥ��Ƶ����64��Ƶ
    hiwdg->Init.Reload = 0xFFF; // ����ֵ��������ֵΪ0xFFF
    elab_assert(HAL_IWDG_Init(hiwdg) == HAL_OK);

    elab_watchdog_register(&me->device, name, &watchdog_driver_ops, me);
}

static void _set_time(elab_watchdog_t *me, uint32_t timeout_ms)
{
    elab_assert(me != NULL);
    elab_assert(timeout_ms > 0 && timeout_ms <= 6 * 1000);//��Χ��0~6��
    
    elab_watchdog_driver_t *driver = (elab_watchdog_driver_t *)me->super.user_data;
    IWDG_HandleTypeDef *hiwdg = &driver->hiwdg;
    
    uint32_t reloadValue = timeout_ms * 40 / (4 * (1 << hiwdg->Init.Prescaler));

    // �������ֵ�Ƿ񳬹�0xFFF
    if (reloadValue > 0xFFF) {
        reloadValue = 0xFFF;
    }

    // ���³�ʼ�����Ź�
    hiwdg->Init.Reload = reloadValue;
    elab_assert(HAL_IWDG_Init(hiwdg) == HAL_OK);
}

static void _feed(elab_watchdog_t *me)
{
    elab_assert(me != NULL);
    
    elab_watchdog_driver_t *driver = (elab_watchdog_driver_t *)me->super.user_data;
    IWDG_HandleTypeDef *hiwdg = &driver->hiwdg;
    
    // ����װ�ؼĴ�����ֵ�ŵ��������У�ι������ֹIWDG��λ
	// ����������ֵ����0��ʱ������ϵͳ��λ
	HAL_IWDG_Refresh(hiwdg);
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
