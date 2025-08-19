/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_device.h"
#include "elab_device_def.h"
#include "../common/elab_assert.h"
#include "../common/elab_log.h"
#include "../os/cmsis_os.h"

ELAB_TAG("ElabDevice");

/* private function prototypes ---------------------------------------------- */
static void _add_device(elab_device_t *me);
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS)
static osMutexId_t _edf_mutex(void);
#endif

/* private variables -------------------------------------------------------- */
static uint32_t _edf_device_count = 0;
static elab_device_t *_edf_table[ELAB_DEV_NUM_MAX];
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS)
static osMutexId_t _mutex_edf = NULL;
#endif

/**
 * The edf global mutex attribute.
 */
 #if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS)
static const osMutexAttr_t _mutex_attr_edf =
{
    .name = "mutex_edf",
    .attr_bits = osMutexPrioInherit | osMutexRecursive,
    .cb_mem = NULL,
    .cb_size = 0, 
};
#endif

/* public function ---------------------------------------------------------- */
/**
 * This function registers a device driver with specified name.
 * @param dev the pointer of device driver structure
 * @param name the device driver's name
 * @return None.
 */
void elab_device_register(elab_device_t *me, elab_device_attr_t *attr)
{
    assert(me != NULL);
    assert(attr != NULL);
    assert(attr->name != NULL);
    assert_name(elab_device_find(attr->name) == NULL, attr->name);
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS)
    /* Edf mutex locking. */
    osStatus_t ret = osOK;
    osMutexId_t mutex = _edf_mutex();
    ret = osMutexAcquire(mutex, osWaitForever);
    assert(ret == osOK);
#endif

    /* Set the device data. */
    memcpy(&me->attr, attr, sizeof(elab_device_attr_t));
    me->enable_count = 0;
    
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS)
    me->lock_count = 0;
    me->mutex = osMutexNew(&_mutex_attr_edf);
    assert(me->mutex != NULL);
#endif

    /* Add the device the edf table. */
    memcpy(&me->attr, attr, sizeof(elab_device_attr_t));
    _add_device(me);

    /* Edf mutex unlocking. */
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS)   
    ret = osMutexRelease(mutex);
    assert(ret == osOK);
#endif
}

/**
 * @brief This function unregisters a device with the device handle.
 * @param me   the pointer of device driver structure
 * @retval None.
 */
void elab_device_unregister(elab_device_t *me)
{
    elab_assert(me != NULL);
    elab_assert(!elab_device_is_enabled(me));
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS)   
    /* Edf mutex locking. */
    osStatus_t ret = osOK;
    osMutexId_t mutex = _edf_mutex();
    ret = osMutexAcquire(mutex, osWaitForever);
    assert(ret == osOK);
#endif

    for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i ++)
    {
        if (_edf_table[i] == me)
        {
            #if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS) 
            osStatus_t ret = osMutexDelete(me->mutex);
            elab_assert(ret == osOK);
            #endif
            me->mutex = NULL;
            _edf_table[i] = NULL;
            _edf_device_count --;
            break;
        }
    }
 
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS) 
    /* Edf mutex unlocking. */
    ret = osMutexRelease(mutex);
    assert(ret == osOK);
#endif
}

/**
 * @brief Get the count number in device framework management.
 * @retval Count number of devices.
 */
uint32_t elab_device_get_number(void)
{
    uint32_t num = 0;
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS) 
    /* Edf mutex locking. */
    osStatus_t ret = osOK;
    osMutexId_t mutex = _edf_mutex();
    ret = osMutexAcquire(mutex, osWaitForever);
    assert(ret == osOK);
#endif

    num = _edf_device_count;

#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS) 
    /* Edf mutex unlocking. */
    ret = osMutexRelease(mutex);
    assert(ret == osOK);
#endif

    return num;
}

/**
 * This function check the given name is the device's name or not.
 * @param me    Device handle.
 * @param name  Device name.
 * @return True or false.
 */
bool elab_device_of_name(elab_device_t *me, const char *name)
{
    bool of_the_name = false;

    elab_device_lock(me);
    if (strcmp(me->attr.name, name) == 0)
    {
        of_the_name = true;
    }
    elab_device_unlock(me);

    return of_the_name;
}

/**
 * This function finds a device driver by specified name.
 * @param name  Device name.
 * @return Device handle. If not found, return NULL.
 */
elab_device_t *elab_device_find(const char *name)
{
    if (_edf_table[0] != NULL)
    assert_name(_edf_table[0]->attr.name != NULL, name);
    assert(name != NULL);
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS)     
    /* Edf mutex locking. */
    osStatus_t ret = osOK;
    osMutexId_t mutex = _edf_mutex();
    ret = osMutexAcquire(mutex, osWaitForever);
    assert(ret == osOK);
#endif
    
    /* Find the device */
    elab_device_t *me = NULL;
    for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i++)
    {
        /* No device yet in the device table. */
        if (_edf_table[i] == NULL)
        {
            break;
        }
        elab_assert(_edf_table[i]->attr.name != NULL);
        /* Device matching */
        if (strcmp(_edf_table[i]->attr.name, name) == 0)
        {
            me = _edf_table[i];
            break;
        }
    }

#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS) 
    /* Edf mutex unlocking. */
    ret = osMutexRelease(mutex);
    assert(ret == osOK);
#endif

    return me;
}

/**
 * This function check one device name is valid or not.
 * @param name  Device name.
 * @return Valid if true and invalid if false.
 */
bool elab_device_valid(const char *name)
{
    return elab_device_find(name) == NULL ? false : true;
}

/**
 * @brief This function check one device name is sole or not.
 * @param name  Device name.
 * @return Valid if true and invalid if false.
 */
bool elab_device_is_sole(elab_device_t *me)
{
    elab_device_lock(me);
    bool enable_status = me->attr.sole;
    elab_device_unlock(me);

    return enable_status;
}

/**
 * @brief Check the device is in test mode or not.
 * @param dev       the pointer of device driver structure
 * @retval True or false.
 */
bool elab_device_is_test_mode(elab_device_t *dev)
{
    return (dev->thread_test != NULL) ? true : false;
}

/**
 * @brief Set the test mode for the device.
 * @param dev       The pointer of device driver structure
 * @retval None.
 */
void elab_device_set_test_mode(elab_device_t *dev)
{
    elab_assert(dev != NULL);

    elab_device_lock(dev);
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS) 
    dev->thread_test = osThreadGetId();
#endif
    elab_device_unlock(dev);
}

/**
 * @brief Set the normal mode for the device.
 * @param dev       the pointer of device driver structure
 * @retval None.
 */
void elab_device_set_normal_mode(elab_device_t *dev)
{
    elab_assert(dev != NULL);

    elab_device_lock(dev);
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS) 
    dev->thread_test = NULL;
#endif
    elab_device_unlock(dev);
}

/**
 * @brief This function check one device is enabled or not.
 * @param name  Device name.
 * @return Valid if true and invalid if false.
 */
bool elab_device_is_enabled(elab_device_t *me)
{
    assert(me != NULL);

    elab_device_lock(me);
    bool enable_status = me->enable_count > 0 ? true : false;
    elab_device_unlock(me);

    return enable_status;
}

/**
 * Lock or unlock one device to ensure its thread-safety.
 * @param me        Device handle.
 * @param status    The locking status.
 * @return None.
 */
void __device_mutex_lock(elab_device_t *me, bool status)
{
    elab_assert(me != NULL);
    
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS) 
    elab_assert(me->mutex != NULL);
    osStatus_t ret = osOK;

    if (status)
    {
        ret = osMutexAcquire(me->mutex, osWaitForever);
    }
    else
    {
        ret = osMutexRelease(me->mutex);
    }
    assert(ret == osOK);
#endif
}

/**
 * This function will enable or disable a device
 * @param me        Device handle.
 * @param status    The locking status.
 * @return the result
 */
elab_err_t __device_enable(elab_device_t *me, bool status)
{
    assert(me != NULL);
    assert(me->ops != NULL);
    assert(me->ops->enable != NULL);

    elab_device_lock(me);
    
    if (me->attr.sole)
    {
        if (status)
        {
            assert_name(me->enable_count == 0, me->attr.name);
        }
        else
        {
            assert_name(me->enable_count > 0, me->attr.name);
        }
    }
    else
    {
        assert_name(me->enable_count < UINT8_MAX, me->attr.name);
    }
    
    elab_err_t ret = ELAB_OK;
    if (status && me->enable_count == 0)
    {
        ret = me->ops->enable(me, true);
    }
    else if (!status && me->enable_count == 1)
    {
        ret = me->ops->enable(me, false);
    }
    me->enable_count = status ? (me->enable_count + 1) : (me->enable_count - 1);

    elab_device_unlock(me);

    return ret;
}

/**
 * This function will read some data from a device.
 *
 * @param dev       the pointer of device driver structure
 * @param buffer    the data buffer to save read data
 * @param size      the size of buffer
 *
 * @return the actually read size on successful, otherwise negative returned.
 *
 */
int32_t elab_device_read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size)
{
    assert(me != NULL);
    assert(me->enable_count != 0);
    assert(me->ops != NULL);
    assert(me->ops->read != NULL);

    int32_t ret = 0;
    if (elab_device_is_test_mode(me))
    {
        ret = ELAB_OK;
        goto exit;
    }

    ret = me->ops->read(me, pos, buffer, size);

exit:
    return ret;
}

/**
 * This function will write some data to a device.
 *
 * @param me        the pointer of device driver structure.
 * @param buffer    the data buffer to be written to device
 * @param size      the size of buffer
 *
 * @return The actually written size on successful, otherwise negative returned.
 *
 */
int32_t elab_device_write(elab_device_t *me,
                            uint32_t pos, const void *buffer, uint32_t size)
{
    assert(me != NULL);
    assert(me->enable_count != 0);
    assert(me->ops != NULL);
    assert(me->ops->write != NULL);

    int32_t ret = 0;
    if (elab_device_is_test_mode(me))
    {
        ret = ELAB_OK;
        goto exit;
    }

    ret = me->ops->write(me, pos, buffer, size);

exit:
    return ret;
}

#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_RTOS) 
/* private functions -------------------------------------------------------- */
static osMutexId_t _edf_mutex(void)
{
    if (_mutex_edf == NULL)
    {
        _mutex_edf = osMutexNew(&_mutex_attr_edf);
        assert(_mutex_edf != NULL);
    }

    return _mutex_edf;
}
#endif

static void _add_device(elab_device_t *me)
{
    assert(_edf_device_count < ELAB_DEV_NUM_MAX);

    if (_edf_device_count == 0)
    {
        for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i ++)
        {
            _edf_table[i] = NULL;
        }
    }
    _edf_table[_edf_device_count ++] = me;
}

/* ----------------------------- end of file -------------------------------- */
