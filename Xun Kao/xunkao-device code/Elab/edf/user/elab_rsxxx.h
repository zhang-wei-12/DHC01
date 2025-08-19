/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */


#ifndef __RS485_H
#define __RS485_H

/* Includes ------------------------------------------------------------------*/
#include "../elab_device.h"
#include "../../os/cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    elab_device_t *serial;
    elab_device_t *pin_tx_en;
    bool tx_en_high_active;
    osMutexId_t mutex;  

    void *user_data;
} RSxxx_t;

/* Exported functions --------------------------------------------------------*/
elab_err_t RSxxx_init(RSxxx_t *me,
                        const char *serial_name,
                        const char *pin_tx_en_name,
                        bool tx_en_high_active,
                        void *user_data);
elab_device_t *RSxxx_get_serial(RSxxx_t *me);

int32_t RSxxx_read(RSxxx_t *me, void *pbuf, uint32_t size);
int32_t RSxxx_write(RSxxx_t *me, const void *pbuf, uint32_t size);
int32_t RSxxx_write_time(RSxxx_t *me, const void *pbuf, uint32_t size, uint32_t time);

#ifdef __cplusplus
}
#endif

#endif /* __RS485_H */

/* ----------------------------- end of file -------------------------------- */
