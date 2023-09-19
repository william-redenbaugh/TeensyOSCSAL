#include "global_includes.h"

typedef enum
{
    OS_TIMER_STATUS_STOPPED = 0,
    OS_TIMER_STATUS_ACTIVE_ONESHOT = 1,
    OS_TIMER_STATUS_ACTIVE = 2
} os_timer_status_t;

#define NUM_MAX_TIMERS 32

typedef struct timer_node
{
    os_timer_t *timer;
} timer_node_t;

static timer_node_t nodes_list[NUM_MAX_TIMERS];
static int num_nodes = 0;

void timer_thread_init(void *parameters)
{
    for (int n = 0; n < NUM_MAX_TIMERS; n++)
    {
        nodes_list[n].timer = NULL;
    }
}

void timer_thread(void *parameters)
{
    for (;;)
    {
        uint32_t next_sleep_ms = 8000;
        for (int n = 0; n < num_nodes; n++)
        {
            if (nodes_list[n].timer->status != OS_TIMER_STATUS_STOPPED)
            {
                if (nodes_list[n].timer->last_updated + nodes_list[n].timer->interval_ms <= millis())
                {
                    // Run timer_callback
                    if (nodes_list[n].timer->timer_cb != NULL)
                    {
                        nodes_list[n].timer->timer_cb(nodes_list[n].timer->params);
                    }

                    // Oneshot timer doing oneshot stuff
                    if (nodes_list[n].timer->status == OS_TIMER_STATUS_ACTIVE_ONESHOT)
                    {
                        nodes_list[n].timer->status = OS_TIMER_STATUS_STOPPED;
                    }
                }

                // When does this timer need to next run;
                uint32_t next_run = nodes_list[n].timer->interval_ms - (millis() - nodes_list[n].timer->last_updated);
                // If this is the next shortest time to be ran we update the next sleep
                if (next_sleep_ms > next_run)
                {
                    next_sleep_ms = next_run;
                }
            }
        }
        os_thread_delay_ms(next_sleep_ms);
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

    int n;
    for (n = 0; n < NUM_MAX_TIMERS; n++)
    {
        if (nodes_list[n].timer == NULL)
        {
            nodes_list[n].timer = timer;
            break;
        }
    }

    // No active timers
    if (n >= NUM_MAX_TIMERS)
    {
        return OS_RET_LOW_MEM_ERROR;
    }

    return OS_RET_OK;
}

int os_timer_start_oneshot(os_timer_t *timer)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    timer->status = OS_TIMER_STATUS_ACTIVE_ONESHOT;
    timer->last_updated = millis();

    return OS_RET_OK;
}

int os_timer_start_recurring(os_timer_t *timer)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    timer->status = OS_TIMER_STATUS_ACTIVE;
    timer->last_updated = millis();

    return OS_RET_OK;
}

int os_timer_interval_change(os_timer_t *timer, int interval_ms)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    timer->interval_ms = interval_ms;
    return OS_RET_OK;
}

int os_timer_stop(os_timer_t *timer)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    timer->status = OS_TIMER_STATUS_STOPPED;
    return OS_RET_OK;
}

int os_timer_deinit(os_timer_t *timer)
{
    if (timer == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    if (num_nodes == 0)
    {
        return OS_RET_INVALID_PARAM;
    }

    int current = 0;
    int high = num_nodes - 1;
    timer->status = OS_TIMER_STATUS_STOPPED;

    for (int n = 0; n < NUM_MAX_TIMERS; n++)
    {
        if (nodes_list[n].timer == timer)
        {
            current = n;
            break;
        }
    }

    // Make sure element in the list
    if (nodes_list[current].timer == timer)
    {
        // Doesn't matter the order, so we just swap and delete
        nodes_list[current].timer = nodes_list[high].timer;
        nodes_list[high].timer = NULL;
        num_nodes--;
        return OS_RET_OK;
    }
    else
    {
        return OS_RET_INVALID_PARAM;
    }
}
