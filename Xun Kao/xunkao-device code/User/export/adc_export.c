
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab/common/elab_export.h"
#include "../driver/drv_adc.h"
#include "elab/common/elab_assert.h"

/* private variables -------------------------------------------------------- */
static elab_adc_driver_t adc_drv;

/* public functions --------------------------------------------------------- */
static void adc_init_export(void)
{
    elab_driver_adc_init(&adc_drv, "ADC_VOL", "C.03", "ADC1", 13);
}
//INIT_EXPORT(adc_init_export, EXPORT_DRVIVER);

/* ----------------------------- end of file -------------------------------- */
