#ifndef _CSAL_TEENSYOS_DEFS_H
#define _CSAL_TEENSYOS_DEFS_H
#include "OS/OSThreadKernel.h"
#include "OS/OSMutexKernel.h"
#include "CSAL_SHARED/unsafe_fifo.h"

typedef struct os_mut_t
{
    volatile uint32_t state = MUTEX_UNLOCKED;
    os_thread_id_t which_thread = -1;
    unsafe_fifo_t thread_queue;
} os_mut_t;

typedef struct os_sem_t
{
    volatile uint32_t count = 0;
    volatile uint32_t max_count = 0;
    unsafe_fifo_t thread_queue;
} os_sem_t;

typedef struct os_setbits_t
{
    unsafe_fifo_t thread_queue;
    volatile uint32_t bits = 0;
} os_setbits_t;

typedef struct os_timer_t
{

} os_timer_t;

typedef void (*os_timer_cb_t)(void *params);

#endif
