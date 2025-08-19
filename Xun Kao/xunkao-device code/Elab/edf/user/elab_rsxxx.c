/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "elab_RSxxx.h"
#include "../../common/elab_log.h"
#include "../../common/elab_assert.h"
#include "../normal/elab_serial.h"
#include "../normal/elab_pin.h"

ELAB_TAG("rs485");

#if (ELAB_RTOS_CMSIS_OS_EN != 0)
static const osMutexAttr_t mutex_attr =
{
    "mutex_elog", osMutexRecursive | osMutexPrioInherit, NULL, 0U 
};
#endif

/* Private function prototypes -----------------------------------------------*/
static void rs485_tx_active(RSxxx_t *me, bool active);

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  rsxxx init function
  * @param  me The RSxxx handle
  * @param  serial_name The input serial device name
  * @param  pin_tx_en_name  The input tx pin name, only for RS485
  * @param  tx_en_high_active The tx enable pin is high-active or not, only for RS485
  * @param  user_data   User private data attached to the rs485 object.
  * @retval See elab_err_t
  */
elab_err_t RSxxx_init(RSxxx_t *me,
                        const char *serial_name,
                        const char *pin_tx_en_name,
                        bool tx_en_high_active,
                        void *user_data)
{
    elab_err_t ret = ELAB_OK;
    uint32_t mode = ELAB_SERIAL_MODE_FULL_DUPLEX;

    me->mutex = osMutexNew(&mutex_attr);
    assert_name(NULL != me->mutex, serial_name);

    me->serial = elab_device_find(serial_name);
    assert_name(NULL != me->serial, serial_name);

    if (pin_tx_en_name != NULL)//rs485
    {
        me->pin_tx_en = elab_device_find(pin_tx_en_name);
        assert_name(NULL != me->pin_tx_en, serial_name);
        me->tx_en_high_active = tx_en_high_active;
        /* Set the rx485 to receiving mode. */
        elab_pin_set_mode(me->pin_tx_en, PIN_MODE_OUTPUT_PP);
        rs485_tx_active(me, false);
        
        /* Serial mode setting. */
        mode = ELAB_SERIAL_MODE_HALF_DUPLEX;
    }
    
    me->user_data = user_data;
    
    /* Serail port opening. */
    ret = elab_device_open(me->serial);
    if (ELAB_OK != ret)
    {
        elog_error("RS485 opening serial %s port fails.", serial_name);
        goto exit;
    }
    
    elab_serial_set_mode(me->serial, mode);

exit:
    return ret;
}

/**
  * @brief  Get serial handle from RS485.
  * @param  me   The RSxxx handle
  * @retval Serail handle.
  */
elab_device_t *RSxxx_get_serial(RSxxx_t *me)
{
    return me->serial;
}

/**
  * @brief  RS485 read function
  * @param  me   The RS485 handle
  * @param  pbuf The pointer of buffer
  * @param  size Expected read length
  * @retval Auctual read length
  */
int32_t RSxxx_read(RSxxx_t *me, void *pbuf, uint32_t size)
{
    elab_serial_t *serial = (elab_serial_t *)me->serial;
    elab_assert(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX 
               || serial->attr.mode == ELAB_SERIAL_MODE_FULL_DUPLEX);
    
    int32_t ret = elab_device_read(me->serial, 0, pbuf, size);

    return ret;
}

/**
  * @brief  RS485 write function
  * @param  me   The RSxxx handle
  * @param  pbuf The pointer of buffer
  * @param  size Expected write length
  * @retval Auctual write length
  */
int32_t RSxxx_write(RSxxx_t *me, const void *pbuf, uint32_t size)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(me->mutex, osWaitForever);
    assert_name(ret_os == osOK, me->serial->attr.name);
    elab_serial_t *serial = (elab_serial_t *)me->serial;
    elab_assert(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX 
               || serial->attr.mode == ELAB_SERIAL_MODE_FULL_DUPLEX);
    
    if(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX)
    {
        /* Set the rx485 to sending mode. */
        rs485_tx_active(me, true);
    }

    int32_t ret = elab_device_write(me->serial, 0, pbuf, size);

    /*  On x86-64 simulator, data are transmitted to the other RSxxx in very short
        time, but on actual boards it takes some time. So on ARM platform, osDelay
        will be invoked here. */
//#if defined(__arm__) && (EALB_SIMU_EN == 0)
//    //osDelayUs((size * 10000000 / serial->attr.baud_rate) + 1000);
//    //osDelay((size * 10000000 / serial->attr.baud_rate) + 1000);
//#endif

    if(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX)
    {
        /* Set the rx485 to receiving mode. */
        rs485_tx_active(me, false);
    }
    
    ret_os = osMutexRelease(me->mutex);
    assert_name(ret_os == osOK, me->serial->attr.name);

    return ret;
}

int32_t RSxxx_write_time(RSxxx_t *me, const void *pbuf, uint32_t size, uint32_t time)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(me->mutex, osWaitForever);
    assert_name(ret_os == osOK, me->serial->attr.name);
    elab_serial_t *serial = (elab_serial_t *)me->serial;
    elab_assert(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX 
               || serial->attr.mode == ELAB_SERIAL_MODE_FULL_DUPLEX);
    
    if(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX)
    {
        /* Set the rx485 to sending mode. */
        rs485_tx_active(me, true);
    }

    int32_t ret = elab_device_write(me->serial, 0, pbuf, size);

    osDelay(time);
    
    if(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX)
    {
        /* Set the rx485 to receiving mode. */
        rs485_tx_active(me, false);
    }
    
    ret_os = osMutexRelease(me->mutex);
    assert_name(ret_os == osOK, me->serial->attr.name);

    return ret;
}

/* Private defines -----------------------------------------------------------*/
/**
  * @brief  RS485 set the tx mode active or not.
  * @param  me      The RS485 handle
  * @param  active  TX mode is active or not.
  * @retval None.
  */
static void rs485_tx_active(RSxxx_t *me, bool active)
{
    // Write the tx_en pin.
    bool tx_en_status = false;
    if (me->tx_en_high_active == true)
    {
        tx_en_status = active ? true : false;
    }
    else
    {
        tx_en_status = active ? false : true;
    }

    elab_pin_set_status(me->pin_tx_en, tx_en_status);
}

/* ----------------------------- end of file -------------------------------- */
