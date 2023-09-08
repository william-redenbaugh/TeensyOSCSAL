#include "global_includes.h"

#define MAX_THREADS_ACQUIRE_SEMAPHORE 8

int os_sem_init(os_sem_t *sem, int count)
{

    if (sem == NULL)
    {
        return OS_RET_NULL_PTR;
    }
}

int os_sem_entry(os_sem_t *sem, uint32_t timeout_ms)
{
    if (sem == NULL)
    {
        return OS_RET_NULL_PTR;
    }
}

int os_sem_count(os_sem_t *sem)
{
    if (sem == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    return sem->count;
}

int os_sem_entry_wait_indefinite(os_sem_t *sem)
{
    if (sem == NULL)
    {
        return OS_RET_NULL_PTR;
    }
    return unsafe_fifo_queue_init(&sem->thread_queue, MAX_THREADS_ACQUIRE_SEMAPHORE, sizeof(os_thread_id_t));
}

int os_sem_give(os_sem_t *sem)
{
    if (sem == NULL)
    {
        return OS_RET_NULL_PTR;
    }
}