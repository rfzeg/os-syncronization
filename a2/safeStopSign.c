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
	sign->carQueue = initIntQueue();

	initMutex(&sign->n_lock);
	initMutex(&sign->s_lock);
	initMutex(&sign->e_lock);
	initMutex(&sign->w_lock);

	initConditionVariable(&sign->n_lane_cv);
	initConditionVariable(&sign->s_lane_cv);
	initConditionVariable(&sign->e_lane_cv);
	initConditionVariable(&sign->w_lane_cv);

	laneMutexArr = malloc(sizeof(pthread_mutex_t) * 4);
	laneCondVarArr = malloc(sizeof(laneCondVarArr) * 4);

	laneMutexArr[0] = &sign->e_lock;
	laneMutexArr[1] = &sign->n_lock;
	laneMutexArr[2] = &sign->w_lock;
	laneMutexArr[3] = &sign->s_lock;

	laneCondVarArr[0] = &sign->e_lane_cv;
	laneCondVarArr[1] = &sign->n_lane_cv;
	laneCondVarArr[2] = &sign->w_lane_cv;
	laneCondVarArr[3] = &sign->s_lane_cv;




}

void destroySafeStopSign(SafeStopSign* sign) {
	destroyStopSign(&sign->base);

	// TODO: Add any logic you need to clean up data structures.
	freeQueue(sign->carQueue);

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

	EntryLane* lane = getLane(car, &sign->base);
	laneNum = car->position;
	carAction = car->action;
	
	enterLane(car, lane);
	enqueue(sign->carQueue, car->index);
	
	goThroughStopSign(car, &sign->base);
	exitCar = dequeue(sign->carQueue);
	exitIntersection(car, lane);
}
