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

/**
 * A node that forms one element of our IntQueue. Has a value, and a pointer to the next node
 */
struct IntQueueNode {
    int val;
    struct IntQueueNode *next;
};

/**
 * A queue data structure that holds integers
 */
struct IntQueue {
    struct IntQueueNode *head;
    struct IntQueueNode *tail;
    int size;
};

typedef struct IntQueue IntQueue_t;
typedef struct IntQueueNode IntQueueNode_t;

/**
 * Allocate memory and start values for a new IntQueue
 *
 * @return a pointer to the newly created IntQueue
 */
struct IntQueue *initIntQueue();

/**
 * Add integer i to the end of queue q
 *
 * @param q the queue
 * @param i the integer
 */
void enqueue(struct IntQueue *q, int i);

/**
 * Remove and return the first element of the queue
 *
 * @param q the queue to operate on
 */
int dequeue(struct IntQueue *q);

/**
 * Free all memory allocated by queue q and its elements
 *
 * @param q the queue to free
 */
void freeQueue(struct IntQueue *q);

/**
* @brief Calls cv wait and performs error checking
*
* @param cond pointer to the cv.
* @param lock pointer to lock
*/
void cvWait(pthread_cond_t *cond, pthread_mutex_t *lock);

/**
* @brief Calls broadcast on cv and performs error checking
*
* @param cond pointer to the cv
*/
void cvBroadcast(pthread_cond_t *cond);

/**
 * Wake up cars that are waiting so they can re-check their condition
 *
 * @param an array of conditional variables to broadcast to
 * @param arrLength the length of the array
 */
void broadcastMultipleLanes(pthread_cond_t *cvArr, int arrLength);
