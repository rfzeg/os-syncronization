#pragma once
/**
* CSC369 Assignment 2
*
* This is the header file for your safe stop sign submission code.
*/
#include "car.h"
#include "stopSign.h"

/**
* @brief Structure that you can modify as part of your solution to implement
* proper synchronization for the stop sign intersection.
*
* This is basically a wrapper around StopSign, since you are not allowed to 
* modify or directly access members of StopSign.
*/
typedef struct _SafeStopSign {

	/**
	* @brief The underlying stop sign.
	*
	* You are not allowed to modify the underlying stop sign or directly
	* access its members. All interactions must be done through the functions
	* you have been provided.
	*/
	StopSign base;

	// TODO: Add any members you need for synchronization here.
	pthread_mutex_t n_lock;
	pthread_mutex_t s_lock;
	pthread_mutex_t e_lock;
	pthread_mutex_t w_lock;

	pthread_cond_t n_lane_cv;
	pthread_cond_t s_lane_cv;
	pthread_cond_t e_lane_cv;
	pthread_cond_t w_lane_cv;

    struct IntQueue *carQueue;



} SafeStopSign;

/**
 * An array to hold mutexes corresponding to each lane
 */
pthread_mutex_t **laneMutexArr;

 /**
 * An array to hold condition variables corresponding to each lane
 */
pthread_cond_t **laneCondVarArr;

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
* @brief Initializes the safe stop sign.
*
* @param sign pointer to the instance of SafeStopSign to be initialized.
* @param carCount number of cars in the simulation.
*/
void initSafeStopSign(SafeStopSign* sign, int carCount);

/**
* @brief Destroys the safe stop sign.
*
* @param sign pointer to the instance of SafeStopSign to be freed
*/
void destroySafeStopSign(SafeStopSign* sign);

/**
* @brief Runs a car-thread in a stop-sign scenario.
*
* @param car pointer to the car.
* @param sign pointer to the stop sign intersection.
*/
void runStopSignCar(Car* car, SafeStopSign* sign);
