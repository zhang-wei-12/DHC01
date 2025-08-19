/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "elab/common/elab_assert.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"
#include "elab/edf/normal/elab_rtc.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("TestLed");

/* private functions -------------------------------------------------------- */
static int32_t test_rtc_set_time(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    uint32_t year, month, day, hour, minute, second, week;
    elab_rtc_time_t rtc_time = {0};
    elab_device_t * rtc = elab_device_find("rtc");
    elab_assert(rtc != NULL);
    
    if (argc != 8)
    {
        elog_error("Not right argument number: %u. It should be 7.", argc);
        ret = -1;
        goto exit;
    }
    
    year = atoi(argv[1]);
    if (year >= 1970 && year <= 2058)
    {
        rtc_time.date.year = year;
    }
    else
    {
        elog_error("Not right year: %s. Should be 1970~2058.", argv[1]);
        ret = -2;
        goto exit;
    }
    
    month = atoi(argv[2]);
    if (month >= 1 && month <= 12)
    {
        rtc_time.date.month = month;
    }
    else
    {
        elog_error("Not right month: %s. Should be 1~12.", argv[2]);
        ret = -3;
        goto exit;
    }
    
    day = atoi(argv[3]);
    if (month >= 1 && month <= 31)
    {
        rtc_time.date.day = day;
    }
    else
    {
        elog_error("Not right month: %s. Should be 1~31.", argv[3]);
        ret = -3;
        goto exit;
    }
    
    hour = atoi(argv[4]);
    if (hour <= 23)
    {
        rtc_time.time.hour = hour;
    }
    else
    {
        elog_error("Not right month: %s. Should be 0~23", argv[4]);
        ret = -4;
        goto exit;
    }
    
    minute = atoi(argv[5]);
    if (minute <= 59)
    {
        rtc_time.time.minute = minute;
    }
    else
    {
        elog_error("Not right minute: %s. Should be 0~59", argv[5]);
        ret = -5;
        goto exit;
    }
    
    second = atoi(argv[6]);
    if (second <= 59)
    {
        rtc_time.time.second = second;
    }
    else
    {
        elog_error("Not right second: %s. Should be 0~59", argv[6]);
        ret = -5;
        goto exit;
    }
    
    week = atoi(argv[7]);
    if (week >= 1 && week <= 7)
    {
        rtc_time.date.week = week;
    }
    else
    {
        elog_error("Not right second: %s. Should be 0~59", argv[6]);
        ret = -5;
        goto exit;
    }
    
    elab_rtc_set_time(rtc, &rtc_time);
    
exit:
    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_rtc_set,
                    test_rtc_set_time,
                    RTC testing function);

static int32_t test_rtc_get_time(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    
    elab_rtc_time_t rtc_time = {0};
    
    elab_device_t * rtc = elab_device_find("rtc");
    elab_assert(rtc != NULL);
    
    elab_rtc_get_time(rtc, &rtc_time);
    elog_debug("Now is %d-%02d-%02d, %02d:%02d:%02d\n",
                rtc_time.date.year, rtc_time.date.month, rtc_time.date.day,
                rtc_time.time.hour, rtc_time.time.minute, rtc_time.time.second);
    
    elog_debug("Weekday is %d\n", rtc_time.date.week);
   
    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_rtc_get,
                    test_rtc_get_time,
                    RTC testing function);




#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
