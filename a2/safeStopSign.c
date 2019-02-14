/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe stop sign
* submission code.
*/
#include "safeStopSign.h"

int quadrantClaims[4] = {-1, -1, -1, -1}; // -1 if the quadrant is not claimed, set to carIndex if claimed

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

void initSafeStopSign(SafeStopSign* sign, int count) {
	initStopSign(&sign->base, count);

	// TODO: Add any initialization logic you need.
	sign->northQueue = initIntQueue();
	sign->southQueue = initIntQueue();
	sign->eastQueue = initIntQueue();
	sign->westQueue = initIntQueue();

	initMutex(&sign->nLock);
	initMutex(&sign->sLock);
	initMutex(&sign->eLock);
	initMutex(&sign->wLock);

	initConditionVariable(&sign->northLaneCV);
	initConditionVariable(&sign->southLaneCV);
	initConditionVariable(&sign->eastLaneCV);
	initConditionVariable(&sign->westLaneCV);

	sign->laneMutexArr = malloc(sizeof(pthread_mutex_t) * 4);
	sign->laneCondVarArr = malloc(sizeof(sign->laneCondVarArr) * 4);

	sign->laneMutexArr[0] = &sign->eLock;
	sign->laneMutexArr[1] = &sign->nLock;
	sign->laneMutexArr[2] = &sign->wLock;
	sign->laneMutexArr[3] = &sign->sLock;

	sign->laneCondVarArr[0] = &sign->eastLaneCV;
	sign->laneCondVarArr[1] = &sign->northLaneCV;
	sign->laneCondVarArr[2] = &sign->westLaneCV;
	sign->laneCondVarArr[3] = &sign->southLaneCV;

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

	pthread_cond_destroy(&sign->northLaneCV);
	pthread_cond_destroy(&sign->southLaneCV);
	pthread_cond_destroy(&sign->eastLaneCV);
	pthread_cond_destroy(&sign->westLaneCV);

	pthread_mutex_destroy(&sign->nLock);
	pthread_mutex_destroy(&sign->sLock);
	pthread_mutex_destroy(&sign->eLock);
	pthread_mutex_destroy(&sign->wLock);

	free(sign->laneMutexArr);
	free(sign->laneCondVarArr);
	free(sign->laneQueues);
}

int claimQuadrants(SafeStopSign* sign, int *quadrants, int numClaims, int carIndex) {
	int quadrantsClaimed = 1;

	for (int i = 0; i < numClaims; i++) {
		if (quadrantClaims[quadrants[i]] == -1) {
			quadrantClaims[quadrants[i]] = carIndex;
		} else {
			// quadrant is being used by another car
			quadrantsClaimed = 0;
			break;
		}
	}
	return quadrantsClaimed;
}

void unclaimQuadrants(SafeStopSign* sign, int *quadrantsClaimed, int numClaims) {
	for (int i = 0; i < numClaims; i++){
		quadrantClaims[quadrantsClaimed[i]] = -1;
	}
}

void broadcastAllLanes(SafeStopSign* sign) {
	for (int i = 0; i < 4; i++) {
		pthread_cond_broadcast(sign->laneCondVarArr[i]);
	}
}

void runStopSignCar(Car* car, SafeStopSign* sign) {
	// TODO: Add your synchronization logic to this function.
	int laneNum;

	int *quadrantsNeeded = malloc(sizeof(int) * QUADRANT_COUNT);
	int quadrantsNeededCount = getStopSignRequiredQuadrants(car,quadrantsNeeded);
	EntryLane* lane = getLane(car, &sign->base);
	laneNum = car->position;
	lock(sign->laneMutexArr[laneNum]);
	enterLane(car, lane);
	enqueue(sign->laneQueues[laneNum], car->index);

	unlock(sign->laneMutexArr[laneNum]);

	lock(&sign->quadrantClaimLock);
	while (!claimQuadrants(sign, quadrantsNeeded, quadrantsNeededCount, car->index)) {
		pthread_cond_wait(sign->laneCondVarArr[laneNum], sign->laneMutexArr[laneNum]);
	}
	unlock(&sign->quadrantClaimLock);

	goThroughStopSign(car, &sign->base);
	lock(&sign->quadrantClaimLock);
	unclaimQuadrants(sign, quadrantsNeeded, quadrantsNeededCount);
	broadcastAllLanes(sign);
	unlock(&sign->quadrantClaimLock);

	lock(sign->laneMutexArr[laneNum]);
	// Ensures cars who entered lane first exits first
	while(sign->laneQueues[laneNum]->size > 0 && car->index != sign->laneQueues[laneNum]->head->val){
		pthread_cond_wait(sign->laneCondVarArr[laneNum], sign->laneMutexArr[laneNum]);
	}
	exitIntersection(car, lane);
	dequeue(sign->laneQueues[laneNum]);
	pthread_cond_broadcast(sign->laneCondVarArr[laneNum]);
	unlock(sign->laneMutexArr[laneNum]);

	free(quadrantsNeeded);
}

void lock(pthread_mutex_t *mutex) {
	int returnValue = pthread_mutex_lock(mutex);
	if (returnValue != 0) {
		perror("Mutex lock failed."
			   "@ " __FILE__ " : " LINE_STRING "\n");
	} else {
		printf("not a retard");
	}

}