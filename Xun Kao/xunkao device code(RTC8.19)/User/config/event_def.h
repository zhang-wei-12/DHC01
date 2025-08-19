/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __EVENT_DEF_H__
#define __EVENT_DEF_H__

/* includes ----------------------------------------------------------------- */
#include "elab/3rd/qpc/include/qpc.h"

/* public typedef ----------------------------------------------------------- */
enum
{
    Q_NULL_SIG = 0,
    Q_QPC_TEST_SIG = Q_USER_SIG,

    EVT_BUTTON_UT_PRESS,
    EVT_BUTTON_UT_RELEASE,
    EVT_BUTTON_UT_CLICK,
    EVT_BUTTON_UT_DOUBLE_CLICK,
    EVT_BUTTON_UT_LONG_PRESS,

    EVT_BUTTON_KEY1,
    EVT_BUTTON_KEY2,

    ECAP_TEST_MIN,
    ECAP_TEST_MAX = ECAP_TEST_MIN + 63,
    
    Q_MAX_PUB_SIG,
    Q_MAX_SIG,                        /* the last signal (keep always last) */
};

#endif

/* ----------------------------- end of file -------------------------------- */
