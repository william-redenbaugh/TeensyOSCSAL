#include "global_includes.h"
#include "OS/OSThreadKernel.h"

#define MAX_THREADS_ACQUIRE_SIGNAL 32

int os_setbits_init(os_setbits_t *mod)
{
    if (mod == NULL)
    {
        return OS_RET_NULL_PTR;
    }
    return unsafe_fifo_queue_init(&mod->thread_queue, MAX_THREADS_ACQUIRE_SIGNAL, sizeof(os_thread_id_t));
}

int os_setbits_signal(os_setbits_t *mod, int bit)
{
    if (mod == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    // Signal bitmask
    mod->bits |= bit;

    int os_state = os_stop();

    while (true)
    {
        // If there's a thread waiting to be resumed by the
        os_thread_id_t thread_id;
        int ret = unsafe_fifo_dequeue(&mod->thread_queue, sizeof(os_thread_id_t), &thread_id);

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
            if (thread_ptr->flags == THREAD_BLOCKED_SIGNAL || thread_ptr->flags == THREAD_BLOCKED_SIGNAL_TIMEOUT)
            {
                // Reboot the threads
                thread_ptr->flags = THREAD_RUNNING;
                break;
            }
        }
        // No thread waiting in line... break outa the loop.
        else
        {
            break;
        }
    }

    // reboot the OS kernel.
    os_start(os_state);

    return OS_RET_OK;
}

int os_clearbits(os_setbits_t *mod, int bit)
{
    if (mod == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    // Clear out all the bits we have in the bitmask
    mod->bits &= (!bit);

    return OS_RET_OK;
}

int os_waitbits(os_setbits_t *mod, uint8_t bit, uint32_t timeout_ms)
{
    if (mod == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    if (mod == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    // Checking case immediatly.
    if (OS_CHECK_BIT(mod->bits, (uint32_t)bit))
        return OS_RET_OK;

    // Stop the operating system so we can make changes to the thread.
    int os_state = os_stop();

    // Throw it into the thread list to get resumed later.
    os_thread_id_t thread_id = os_current_id();

    unsafe_fifo_enqueue(&mod->thread_queue, sizeof(os_thread_id_t), &thread_id);
    thread_t *this_thread = _os_current_thread();

    // Set the bit that the thread is waiting on
    this_thread->signal_bits_compare = bit;
    this_thread->flags = THREAD_BLOCKED_SIGNAL_TIMEOUT;
    this_thread->interval = timeout_ms;
    this_thread->previous_millis = millis();

    // Restart the OS after we have completed touching the thread.
    os_start(os_state);

    // It's easiest to just resume the thread, check if those bits were the right bits
    // Then stop if so.
    while (true)
    {
        // Context switch out of the thread. Whatever happens happens.
        _os_yield();

        os_state = os_stop();

        // We experienced a timeout...
        if ((this_thread->interval + this_thread->previous_millis) < millis())
        {
            // Restart the OS after we have completed touching the thread.
            os_start(os_state);
            return OS_RET_TIMEOUT;
        }

        int bitmask = this_thread->signal_bits_compare | mod->bits;

        // Somewone set the right bit!
        if (bitmask)
        {
            // Restart the OS after we have completed touching the thread.
            os_start(os_state);
            return OS_RET_OK;
        }
        else
        {
            // The bitmask was wrong, so we throw it back into the circular
            this_thread->flags = THREAD_BLOCKED_SIGNAL;

            // Next person to setbits knows what's up
            unsafe_fifo_enqueue(&mod->thread_queue, sizeof(os_thread_id_t), &thread_id);
            // Restart the OS.
            os_start(os_state);
        }
    }
}

int os_waitbits_indefinite(os_setbits_t *mod, int bit)
{
    if (mod == NULL)
    {
        return OS_RET_NULL_PTR;
    }

    // Checking case immediatly.
    if (OS_CHECK_BIT(mod->bits, (uint32_t)bit))
        return OS_RET_OK;

    // Stop the operating system so we can make changes to the thread.
    int os_state = os_stop();

    // Throw it into the thread list to get resumed later.
    os_thread_id_t thread_id = os_current_id();

    unsafe_fifo_enqueue(&mod->thread_queue, sizeof(os_thread_id_t), &thread_id);
    thread_t *this_thread = _os_current_thread();

    // Set the bit that the thread is waiting on
    this_thread->signal_bits_compare = bit;
    this_thread->flags = THREAD_BLOCKED_SIGNAL;

    // Restart the OS after we have completed touching the thread.
    os_start(os_state);

    // It's easiest to just resume the thread, check if those bits were the right bits
    // Then stop if so.
    while (true)
    {
        // Context switch out of the thread. Whatever happens happens.
        _os_yield();

        os_state = os_stop();
        int bitmask = this_thread->signal_bits_compare | mod->bits;

        if (bitmask)
        {
            // Restart the OS after we have completed touching the thread.
            os_start(os_state);
            return OS_RET_OK;
        }
        else
        {
            // The bitmask was wrong, so we throw it back into the circular
            this_thread->flags = THREAD_BLOCKED_SIGNAL;

            // Next person to setbits knows what's up
            unsafe_fifo_enqueue(&mod->thread_queue, sizeof(os_thread_id_t), &thread_id);
            os_start(os_state);
        }
    }
}