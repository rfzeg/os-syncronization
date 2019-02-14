#include "common.h"

/**
* @brief Destroys a mutex and does error checking.
*
* @param mutex pointer to the mutex to initialize.
*/
void destroyMutex(pthread_mutex_t* mutex);

/**
* @brief Destroys a condition variable and does error checking.
*
* @param cond pointer to the condition variable to initialize.
*/
void destroyConditionVariable(pthread_cond_t* cond);

/**
* @brief Locks a mutex and does error checking.
*
* @param mutex pointer to the mutex to lock.
*/
void lock(pthread_mutex_t *mutex);