#include "global_includes.h"

typedef enum
{
    OS_TIMER_STATUS_STOPPED = 0,
    OS_TIMER_STATUS_ACTIVE_ONESHOT = 1,
    OS_TIMER_STATUS_ACTIVE = 2
} os_timer_status_t;

void timer_thread_init(void *parameters)
{
}

void timer_thread(void *parameters)
{
    for (;;)
    {
        uint32_t next_sleep_ms = 0;
    }
}

int os_timer_init(os_timer_t *timer, os_timer_cb_t cb, int interval_ms, void *params)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    timer->params = params;
    timer->timer_cb = cb;
    timer->last_updated = millis();
    timer->interval_ms = interval_ms;
    timer->status = OS_TIMER_STATUS_STOPPED;
    return OS_RET_OK;
}

int os_timer_start_oneshot(os_timer_t *timer)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    return OS_RET_OK;
}

int os_timer_start_recurring(os_timer_t *timer)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }
    return OS_RET_OK;
}

int os_timer_interval_change(os_timer_t *timer, int interval_ms)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }
    return OS_RET_OK;
}

int os_timer_stop(os_timer_t *timer)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }
    return OS_RET_OK;
}

int os_timer_deinit(os_timer_t *timer)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }
    return OS_RET_OK;
}
