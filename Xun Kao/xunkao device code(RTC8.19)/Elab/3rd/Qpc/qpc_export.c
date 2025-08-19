/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "event_def.h"
#include "include/qpc.h"
#include "elab/elab.h"

ELAB_TAG("QpExport");

/* Private defines -----------------------------------------------------------*/
#define QPC_TIMER_PERIOD_MS                     (10)

/* Private variables ---------------------------------------------------------*/
static uint32_t time_ms_backup;
#if (ELAB_QPC_EN != 0)
static elab_event_t event_poll[ELAB_EVENT_POOL_SIZE];
#endif

/* Exported functions --------------------------------------------------------*/
static void qpc_export(void)
{
#if (ELAB_QPC_EN != 0)
    /* Initialize the qpc framework. */
    QF_init();

    /* Initialize publish-subscribe table. */
    static QSubscrList subscrSto[Q_MAX_PUB_SIG];
    QF_psInit(subscrSto, Q_DIM(subscrSto));
    
    /* Initialize event pool. */
    QF_poolInit(event_poll, sizeof(event_poll), sizeof(elab_event_t));

    QF_run();
#endif
}
INIT_EXPORT(qpc_export, 0);

/* Exported functions --------------------------------------------------------*/
/**
 * @brief  QP assert callback function.
 * @param  module   QPC software module name.
 * @param  location QPC assert ID.
 */
void Q_onAssert(char_t const * const module, int_t const location)
{
    printf("Q_onAssert module: %s, location: %u.\n", module, location);

    elab_assert(false);

#if defined(__linux__)
    exit(0);
#endif
}

/**
 * @brief  QPC QF_onClockTick callback function.
 */
void QF_onClockTick(void)
{
    /* NULL */
}

/**
 * @brief  QPC QF_onCleanup callback function.
 */
void QF_onCleanup(void)
{
    /* NULL */
}

/**
 * @brief  QPC QF_onStartup callback function.
 */
void QF_onStartup(void)
{
    /* NULL */
}

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  QPC timer thread function.
 */
static void qpc_timer_poll(void)
{
    uint32_t system_time = elab_time_ms();
    uint32_t time_interval = system_time - time_ms_backup;
    for (uint32_t i = 0; i < time_interval; i++)
    {
        QF_tickX_(0);
    }
    time_ms_backup = system_time;
}

#if (ELAB_QPC_EN != 0)
POLL_EXPORT(qpc_timer_poll, QPC_TIMER_PERIOD_MS);
#endif

/* ----------------------------- end of file -------------------------------- */
