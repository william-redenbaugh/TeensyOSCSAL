#include "OS/OSThreadKernel.h"
#include "os_thread.h"
#include "global_includes.h"

/**
 * @brief Linkedlist containing all the idle handler functions we want to run
 */
typedef struct idle_event_handler_node
{
    idle_handler_func idle_func;
    void *parameters;
    struct idle_event_handler_node *next;
} idle_event_handler_node_t;

static idle_event_handler_node_t *head = NULL;

int add_idle_events(idle_handler_func func, void *parameters)
{
    idle_event_handler_node_t *node = head;
    if (head == NULL)
    {
        head = (idle_event_handler_node_t *)malloc(sizeof(idle_event_handler_node_t));
        node = head;
    }
    else
    {
        while (node->next != NULL)
        {
            node = node->next;
        }

        node = (idle_event_handler_node_t *)malloc(sizeof(idle_event_handler_node_t));
    }

    if (node == NULL)
    {
        return OS_RET_LOW_MEM_ERROR;
    }

    node->idle_func = func;
    node->parameters = parameters;

    return OS_RET_OK;
}

/*!
 * @brief Thread that calculates remainder stuff, and sits around
 * @param void *params
 */
void idle_thread_handler(void *params)
{
    for (;;)
    {
        idle_event_handler_node_t *node = head;

        // Run callback functions
        while (node != NULL)
        {
            node->idle_func(node->parameters);
            node = node->next;
        }
        _os_yield();
    }
}