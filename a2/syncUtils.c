#include "syncUtils.h"

void lock(pthread_mutex_t *mutex) {
    int returnValue = pthread_mutex_lock(mutex);
    if (returnValue != 0) {
        perror("Mutex lock failed."
               "@ " __FILE__ " : " LINE_STRING "\n");
    }
}

void destroyMutex(pthread_mutex_t* mutex) {
    int returnValue = pthread_mutex_destroy(mutex);
    if (returnValue != 0) {
        perror("Mutex destruction failed."
               "@ " __FILE__ " : " LINE_STRING "\n");
    }
}

void destroyConditionVariable(pthread_cond_t* cond) {
    int returnValue = pthread_cond_destroy(cond);
    if (returnValue != 0) {
        perror("Condition variable destruction failed."
               "@ " __FILE__ " : " LINE_STRING "\n");
    }
}