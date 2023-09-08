#include "global_includes.h"

#define MAX_THREADS_ACQUIRE_SEMAPHORE 12

int os_sem_init(os_sem_t *sem, int count)
{
    if (sem == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    sem->max_count = count;
    sem->count = 0;

    return unsafe_fifo_queue_init(&sem->thread_queue, MAX_THREADS_ACQUIRE_SEMAPHORE, sizeof(os_thread_id_t));
}

int os_sem_try_entry(os_sem_t *sem)
{
    if (sem == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    // Current state of the operating system.
    int os_state = os_stop();

    if (sem->count == sem->max_count)
    {
        // We are done dealing with OS specific commands
        os_start(os_state);
        return OS_RET_MAX_RENTRANT;
    }

    // Increment the counter
    sem->count++;
    // We are done dealing with OS specific commands
    os_start(os_state);

    return OS_RET_OK;
}

int os_sem_entry(os_sem_t *sem, uint32_t timeout_ms)
{
    if (sem == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    if (os_sem_try_entry(sem) == OS_RET_OK)
    {
        return OS_RET_OK;
    }

    // Stop the kernel for mission critical stuff.
    int os_state = os_stop();

    // Set the state of this thread to being blocked by the mutex
    thread_t *this_thread = _os_current_thread();
    this_thread->semaphore_max_count = sem->max_count;
    this_thread->mutex_semaphore = &sem->count;

    // Set timeout value
    this_thread->flags = THREAD_BLOCKED_SEMAPHORE_TIMEOUT;
    this_thread->interval = timeout_ms;
    this_thread->previous_millis = millis();

    // Enqueue this thread into the list of threads waiting on the mutex
    os_thread_id_t thread_id = os_current_id();
    unsafe_fifo_enqueue(&sem->thread_queue, sizeof(os_thread_id_t), &thread_id);

    // reboot the OS kernel.
    os_start(os_state);

    // Context switch out of the thread.
    // This thread will only resume once it's aqcuired the mutex
    _os_yield();

    // Stop the kernel for mission critical stuff.
    os_state = os_stop();
    // Indicates we timed out instead.
    if (this_thread->interval + this_thread->previous_millis < millis())
    {
        // reboot the OS kernel.
        os_start(os_state);
        return OS_RET_MAX_RENTRANT;
    }

    // reboot the OS kernel.
    os_start(os_state);
    // Otherwise we chilin
    return OS_RET_OK;
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

    if (sem == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    if (os_sem_try_entry(sem) == OS_RET_OK)
    {
        return OS_RET_OK;
    }

    // Stop the kernel for mission critical stuff.
    int os_state = os_stop();

    // Set the state of this thread to being blocked by the mutex
    thread_t *this_thread = _os_current_thread();
    this_thread->semaphore_max_count = sem->max_count;
    this_thread->mutex_semaphore = &sem->count;

    // Disable thread based off flags until it gets scheduled
    this_thread->flags = THREAD_BLOCKED_SEMAPHORE;

    // Enqueue this thread into the list of threads waiting on the mutex
    os_thread_id_t thread_id = os_current_id();
    unsafe_fifo_enqueue(&sem->thread_queue, sizeof(os_thread_id_t), &thread_id);

    // reboot the OS kernel.
    os_start(os_state);

    // Context switch out of the thread.
    // This thread will only resume once it's aqcuired the mutex
    _os_yield();

    return OS_RET_OK;
}

int os_sem_give(os_sem_t *sem)
{
    if (sem == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    int os_state = os_stop();

    __flush_cpu_pipeline();
    os_start(os_state);

    // We are gonna need to make sure whatever threads are waitin
    // On this mutex can get scheduled to run it.
    // If the thread in the queue timed out before we released we move on to next thread
    while (true)
    {
        // If there's a thread waiting to be resumed by the
        os_thread_id_t thread_id;
        int ret = unsafe_fifo_dequeue(&sem->thread_queue, sizeof(os_thread_id_t), &thread_id);

        // Looks like there's a thread waitin in line...
        if (ret == OS_RET_OK)
        {
            thread_t *thread_ptr = os_get_indexed_thread(thread_id);
            // This should never happen, but if we somehow
            // Have an unmapped thread id in the mutex
            if (thread_ptr == NULL)
            {
                continue;
            }

            // Resume the thread
            // There's now a free mutex
            // And a thread that wants it
            // Hopefully whenever this thread resumes it can get it.
            if (thread_ptr->flags == THREAD_BLOCKED_MUTEX || thread_ptr->flags == THREAD_BLOCKED_MUTEX_TIMEOUT)
            {
                //
                thread_ptr->flags = THREAD_RUNNING;
                break;
            }
        }
        // No thread waiting in line... break outa the loop.
        else
        {
            // We actually decrement the counter if this is the case
            if (sem->count > 0)
            {
                sem->count--;
            }
            break;
        }
    }

    return OS_RET_OK;
}