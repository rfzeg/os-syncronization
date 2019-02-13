/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe stop sign
* submission code.
*/
#include "safeStopSign.h"


IntQueue_t *initIntQueue(){
	IntQueue_t *int_queue = malloc(sizeof(IntQueue_t));
	int_queue->head = NULL;
	int_queue->tail = NULL;
	int_queue->size = 0;
	return int_queue;
}

void enqueue(IntQueue_t *q, int i){
	IntQueueNode_t *newElement = malloc(sizeof(IntQueueNode_t));
	newElement->val = i;
	newElement->next = NULL;

	if (q->head == NULL) {
		q->head = newElement;
		q->tail = newElement;
	} else {
		q->tail->next = newElement;
	}
	q->size += 1;
}

int dequeue(IntQueue_t *q){
	int headVal = - 1;
	if (q->size > 0) {
		IntQueueNode_t *head = q->head;
		q->head = q->head->next;
		q->size -= 1;

		headVal = head->val;
		free(head);
	}
	return headVal;
}

void freeQueue(IntQueue_t *q){
	if (q != NULL){
		while (q->size != 0) {
			dequeue(q);
		}
	}
	free(q);
}
int QUADRANT[4] = {0,0,0,0};
void initSafeStopSign(SafeStopSign* sign, int count) {
	initStopSign(&sign->base, count);

	// TODO: Add any initialization logic you need.
	sign->northQueue = initIntQueue();
	sign->southQueue = initIntQueue();
	sign->eastQueue = initIntQueue();
	sign->westQueue = initIntQueue();

	initMutex(&sign->n_lock);
	initMutex(&sign->s_lock);
	initMutex(&sign->e_lock);
	initMutex(&sign->w_lock);

	initConditionVariable(&sign->n_lane_cv);
	initConditionVariable(&sign->s_lane_cv);
	initConditionVariable(&sign->e_lane_cv);
	initConditionVariable(&sign->w_lane_cv);

	sign->laneMutexArr = malloc(sizeof(pthread_mutex_t) * 4);
	sign->laneCondVarArr = malloc(sizeof(sign->laneCondVarArr) * 4);

	sign->laneMutexArr[0] = &sign->e_lock;
	sign->laneMutexArr[1] = &sign->n_lock;
	sign->laneMutexArr[2] = &sign->w_lock;
	sign->laneMutexArr[3] = &sign->s_lock;

	sign->laneCondVarArr[0] = &sign->e_lane_cv;
	sign->laneCondVarArr[1] = &sign->n_lane_cv;
	sign->laneCondVarArr[2] = &sign->w_lane_cv;
	sign->laneCondVarArr[3] = &sign->s_lane_cv;

	sign->laneQueues = malloc(sizeof(IntQueue_t *) * 4);
	sign->laneQueues[0] = sign->eastQueue;
	sign->laneQueues[1] = sign->northQueue;
	sign->laneQueues[2] = sign->westQueue;
	sign->laneQueues[3] = sign->southQueue;

}

void destroySafeStopSign(SafeStopSign* sign) {
	destroyStopSign(&sign->base);

	// TODO: Add any logic you need to clean up data structures.
	freeQueue(sign->northQueue);
	freeQueue(sign->southQueue);
	freeQueue(sign->eastQueue);
	freeQueue(sign->westQueue);

	pthread_cond_destroy(&sign->n_lane_cv);
	pthread_cond_destroy(&sign->s_lane_cv);
	pthread_cond_destroy(&sign->e_lane_cv);
	pthread_cond_destroy(&sign->w_lane_cv);

	pthread_mutex_destroy(&sign->n_lock);
	pthread_mutex_destroy(&sign->s_lock);
	pthread_mutex_destroy(&sign->e_lock);
	pthread_mutex_destroy(&sign->w_lock);
}


void runStopSignCar(Car* car, SafeStopSign* sign) {
	// TODO: Add your synchronization logic to this function.
	int laneNum, exitCar, carAction;

	int quadrantsNeeded[3] = {0,0,0};
	int quadrantCount = getStopSignRequiredQuadrants(car,quadrantsNeeded);
	EntryLane* lane = getLane(car, &sign->base);
	laneNum = car->position;
	carAction = car->action;
	int ret = pthread_mutex_lock(sign->laneMutexArr[laneNum]);
	if (ret != 0){
		perror("Mutex lock failed."
				"@ " __FILE__ " : " LINE_STRING "\n");
	}
	enterLane(car, lane);
	// enqueue(sign->carQueue, car->index);
	
	goThroughStopSign(car, &sign->base);
	// exitCar = dequeue(sign->carQueue);
	exitIntersection(car, lane);

	unlock(sign->laneMutexArr[laneNum]);
}
