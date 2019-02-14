/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe stop sign
* submission code.
*/
#include "safeStopSign.h"

int quadrantClaims[4] = {-1, -1, -1, -1}; // -1 if the quadrant is not claimed, set to carIndex if claimed

void initSafeStopSign(SafeStopSign* sign, int count) {
	initStopSign(&sign->base, count);

	// TODO: Add any initialization logic you need.
	initMutex(&sign->nLock);
	initMutex(&sign->sLock);
	initMutex(&sign->eLock);
	initMutex(&sign->wLock);
	initMutex(&sign->quadrantClaimLock);

	initConditionVariable(&sign->northLaneCV);
	initConditionVariable(&sign->southLaneCV);
	initConditionVariable(&sign->eastLaneCV);
	initConditionVariable(&sign->westLaneCV);

	sign->laneMutexArr = malloc(sizeof(pthread_mutex_t *) * 4);
	sign->laneCondVarArr = malloc(sizeof(pthread_cond_t *) * 4);

	sign->laneMutexArr[0] = &sign->eLock;
	sign->laneMutexArr[1] = &sign->nLock;
	sign->laneMutexArr[2] = &sign->wLock;
	sign->laneMutexArr[3] = &sign->sLock;

	sign->laneCondVarArr[0] = &sign->eastLaneCV;
	sign->laneCondVarArr[1] = &sign->northLaneCV;
	sign->laneCondVarArr[2] = &sign->westLaneCV;
	sign->laneCondVarArr[3] = &sign->southLaneCV;
}

void destroySafeStopSign(SafeStopSign* sign) {
	destroyStopSign(&sign->base);

	// TODO: Add any logic you need to clean up data structures.

	destroyConditionVariable(&sign->northLaneCV);
	destroyConditionVariable(&sign->southLaneCV);
	destroyConditionVariable(&sign->eastLaneCV);
	destroyConditionVariable(&sign->westLaneCV);

	destroyMutex(&sign->quadrantClaimLock);

	destroyMutex(&sign->nLock);
	destroyMutex(&sign->sLock);
	destroyMutex(&sign->eLock);
	destroyMutex(&sign->wLock);

	free(sign->laneMutexArr);
	free(sign->laneCondVarArr);
}

int claimQuadrants(SafeStopSign* sign, int *quadrants, int numClaims, int carIndex) {
	int quadrantsClaimSuccessful = 1;

	for (int i = 0; i < numClaims; i++) {
		if (quadrantClaims[quadrants[i]] == -1) {
			quadrantClaims[quadrants[i]] = carIndex;
		} else {
			// quadrant is being used by another car
			quadrantsClaimSuccessful = 0;

			// Reset all the quadrants we may have claimed before discovering that quadrantClaims[i] is being used
			unclaimQuadrants(sign, carIndex);
			break;
		}
	}

	return quadrantsClaimSuccessful;
}

void unclaimQuadrants(SafeStopSign* sign, int carIndex) {
	for (int i = 0; i < QUADRANT_COUNT; i++){
		if (quadrantClaims[i] == carIndex) { // if the car is currently reserving quadrant i, unreserve it
			quadrantClaims[i] = -1;
		}
	}
}

void broadcastAllLanes(SafeStopSign* sign) {
	for (int i = 0; i < QUADRANT_COUNT; i++) {
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

	// Only one car should be able to 'claim' quadrants it needs to make an action (ie. modify our quadrantClaims arr)
	lock(&sign->quadrantClaimLock);
	// The car should claim all the quadrants it needs (no one else can be using it)
	while (!claimQuadrants(sign, quadrantsNeeded, quadrantsNeededCount, car->index)) {
		pthread_cond_wait(sign->laneCondVarArr[laneNum], &sign->quadrantClaimLock);
	}
	unlock(&sign->quadrantClaimLock);

	goThroughStopSign(car, &sign->base);

	lock(&sign->quadrantClaimLock);
	unclaimQuadrants(sign, car->index);

	// new quadrants have been freed up. wake up all car threads and tell them to re-check if they can claimQuadrants
	broadcastAllLanes(sign);
	
	unlock(&sign->quadrantClaimLock);	

	exitIntersection(car, lane);

	//as we lock the lane until the car exits, we ensure order of cars who exit
	unlock(sign->laneMutexArr[laneNum]);


	free(quadrantsNeeded);
}

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