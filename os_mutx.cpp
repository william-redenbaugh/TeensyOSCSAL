#include "global_includes.h"

#define MAX_THREADS_ACQUIRE_MUTEX 24

int os_mut_init(os_mut_t *mut)
{
    if (mut == NULL)
    {
        return OS_RET_NULL_PTR;
    }
    return unsafe_fifo_queue_init(&mut->thread_queue, MAX_THREADS_ACQUIRE_MUTEX, sizeof(os_thread_id_t));
}

int os_mut_deinit(os_mut_t *mut)
{
    if (mut == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    unsafe_fifo_deinit(&mut->thread_queue);
    return OS_RET_OK;
}

int os_mut_entry(os_mut_t *mut, uint32_t timeout_ms)
{
    if (mut == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    // Try to grab mutex without any pain
    if (os_mut_try_entry(mut) == OS_RET_OK)
    {
        return OS_RET_OK;
    }
    // So you've chosen violence...

    // Stop the kernel for mission critical stuff.
    int os_state = os_stop();

    // Set the state of this thread to being blocked by the mutex
    thread_t *this_thread = _os_current_thread();
    this_thread->mutex_semaphore = &mut->state;
    this_thread->flags = THREAD_BLOCKED_MUTEX_TIMEOUT;

    // Enqueue this thread into the list of threads waiting on the mutex
    os_thread_id_t thread_id = os_current_id();
    unsafe_fifo_enqueue(&mut->thread_queue, sizeof(os_thread_id_t), &thread_id);

    // reboot the OS kernel.
    os_start(os_state);

    // Context switch out of the thread.
    // This thread will only resume once it's aqcuired the mutex
    _os_yield();

    // Since we might get waken up by a timeout
    // Need to catch that.
    if (thread_id == mut->which_thread)
    {
        return OS_RET_OK;
    }
    else
    {
        return OS_RET_MAX_RENTRANT;
    }
}

int os_mut_try_entry(os_mut_t *mut)
{
    // Current state of the operating system.
    int os_state = os_stop();

    // If the lock in unlocked, then we acquire it.
    if (mut->state == MUTEX_UNLOCKED)
    {
        // Let's acquire it!
        mut->state = MUTEX_LOCKED;
        // Mutexes allow only owner thread to keep resources
        mut->which_thread = os_current_id();
        // We are done dealing with OS specific commands
        os_start(os_state);
        // We gottem
        return OS_RET_OK;
    }

    // Otherwise we lose it,
    os_start(os_state);
    // And we fail it.
    return OS_RET_MAX_RENTRANT;
}

int os_mut_count(os_mut_t *mut)
{
    if (mut == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    // Bitwise manipulation is great
    return mut->state & 1;
}

int os_mut_entry_wait_indefinite(os_mut_t *mut)
{
    if (mut == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    // Try to grab mutex without any pain
    if (os_mut_try_entry(mut) == OS_RET_OK)
    {
        return OS_RET_OK;
    }
    // So you've chosen violence...

    // Stop the kernel for mission critical stuff.
    int os_state = os_stop();

    // Set the state of this thread to being blocked by the mutex
    thread_t *this_thread = _os_current_thread();
    this_thread->mutex_semaphore = &mut->state;
    this_thread->flags = THREAD_BLOCKED_MUTEX;

    // Enqueue this thread into the list of threads waiting on the mutex
    os_thread_id_t thread_id = os_current_id();
    unsafe_fifo_enqueue(&mut->thread_queue, sizeof(os_thread_id_t), &thread_id);

    // reboot the OS kernel.
    os_start(os_state);

    // Context switch out of the thread.
    // This thread will only resume once it's aqcuired the mutex
    _os_yield();
}

int os_mut_exit(os_mut_t *mut)
{
    if (mut == NULL)
    {
        return OS_RET_NULL_PTR;
    }
    int os_state = os_stop();

    if (os_current_id() != mut->which_thread)
    {
        os_start(os_state);
        return OS_RET_NOT_OWNED;
    }

    mut->state = MUTEX_UNLOCKED;

    __flush_cpu_pipeline();
    os_start(os_state);

    // We are gonna need to make sure whatever threads are waitin
    // On this mutex can get scheduled to run it.
    while (true)
    {
        // If there's a thread waiting to be resumed by the
        os_thread_id_t thread_id;
        int ret = unsafe_fifo_dequeue(&mut->thread_queue, sizeof(os_thread_id_t), &thread_id);

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
                // Give the mutex ownership to new incoming thread
                mut->which_thread = thread_id;
                mut->state = MUTEX_LOCKED;
                break;
            }
        }
        // No thread waiting in line... break outa the loop.
        else
        {
            break;
        }
    }

    return OS_RET_OK;
}