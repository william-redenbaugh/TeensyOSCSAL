#ifndef _OS_THREAD_H
#define _OSH_THREAD_H

typedef void (*idle_handler_func)(void *parameters);

/**
 * @brief Any events we want to add to the idle event handler we can add here.
 * @note Can only add before scheduler has started
 */
int add_idle_events(idle_handler_func func, void *parameters);

#endif