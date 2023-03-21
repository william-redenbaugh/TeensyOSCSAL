#ifndef _OS_MUTX_H
#define _OS_MUTX_H

#include "os_shared.h"

#ifdef USE_TEENSYOS
#include "OS/OSMutexKernel.h"
#endif

#ifdef USE_NUTTX_POSX
#include "pthread.h"
#endif

typedef struct os_muttx{
#ifdef USE_TEENSYOS
    MutexLock muttx;
#endif

#ifdef USE_NUTTX_POSX

#endif
}os_muttx_t;

enum os_muttx_status_t{
    OS_MUTTX_FREE = 0,
    OS_MUTTX_ACQUIRE = 1
};

/**
 * @brief Initialize Mutex;
 * @param os_muttx_t *muttx pointer to our mutex;
*/
void init_os_muttx(os_muttx_t *muttx);

/**
 * @brief Tries to get and waits on mutexes
 * @param os_muttx_t *muttx pointer to our mutex;
 * @param int timeout of the operating system
*/
void acquire_wait_os_muttx(os_muttx_t *muttx, int timeout);

/**
 * @returns The current status of our Mutex
 * @param os_muttx_t *muttx pointer to our mutex;
*/ 
os_muttx_status_t os_muttx_status(os_muttx_t *muttx);

#endif