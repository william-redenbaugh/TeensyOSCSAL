#ifndef _OS_THREAD_H
#define _OS_THREAD_H

typedef void (*idle_handler_func)(void *parameters);

/**
 * @brief Any events we want to add to the idle event handler we can add here.
 * @note Can only add before scheduler has started
 */
int add_idle_events(idle_handler_func func, void *parameters);

/**
 * @brief True if two task IDs are equal.
 * @note can't check directly because of platform dependant structures
*/
bool os_cmp_id(os_thread_id_t thread_one_id, os_thread_id_t thread_two_id);

#endif