/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe stop sign
* submission code.
*/

#include "safeStopSign.h"
#include "syncUtils.h"

int quadrantClaims[4] = {-1, -1, -1, -1}; // -1 if the quadrant is not claimed, set to carIndex if claimed

void initSafeStopSign(SafeStopSign* sign, int count) {
	initStopSign(&sign->base, count);
	// TODO: Add any initialization logic you need.
	int i;
	for (i = 0; i < QUADRANT_COUNT; i++){
		initMutex(&sign->laneMutexes[i]);
		initConditionVariable(&sign->laneCVs[i]);
	}
	initMutex(&sign->quadrantClaimLock);
}

void destroySafeStopSign(SafeStopSign* sign) {
	destroyStopSign(&sign->base);
	// TODO: Add any logic you need to clean up data structures.
	int i;
	for (i = 0; i < QUADRANT_COUNT; i++){
		destroyMutex(&sign->laneMutexes[i]);
		destroyConditionVariable(&sign->laneCVs[i]);
	}
	destroyMutex(&sign->quadrantClaimLock);
}

int claimQuadrants(int *quadrants, int numClaims, int carIndex) {
	int quadrantsClaimSuccessful = 1;
	for (int i = 0; i < numClaims; i++) {
		if (quadrantClaims[quadrants[i]] == -1) {
			quadrantClaims[quadrants[i]] = carIndex;
		} else {
			// quadrant is being used by another car
			quadrantsClaimSuccessful = 0;
			// Reset all the quadrants we may have claimed before discovering that quadrantClaims[i] is being used
			unclaimQuadrants(carIndex);
			break;
		}
	}
	return quadrantsClaimSuccessful;
}

void unclaimQuadrants(int carIndex) {
	for (int i = 0; i < QUADRANT_COUNT; i++){
		if (quadrantClaims[i] == carIndex) { // if the car is currently reserving quadrant i, unreserve it
			quadrantClaims[i] = -1;
		}
	}
}

void runStopSignCar(Car* car, SafeStopSign* sign) {
	// TODO: Add your synchronization logic to this function.
	int laneNum;
	int *quadrantsNeeded = malloc(sizeof(int) * QUADRANT_COUNT);
	int quadrantsNeededCount = getStopSignRequiredQuadrants(car,quadrantsNeeded);

	EntryLane* lane = getLane(car, &sign->base);
	laneNum = car->position;

	lock(&sign->laneMutexes[laneNum]);
	enterLane(car, lane);
	// Only one car should be able to 'claim' quadrants it needs to make an action (ie. modify our quadrantClaims arr)
	lock(&sign->quadrantClaimLock);
	// The car should claim all the quadrants t needs (no one else can be using it)
	while (!claimQuadrants(quadrantsNeeded, quadrantsNeededCount, car->index)) {
		cvWait(&sign->laneCVs[laneNum], &sign->quadrantClaimLock);
	}
	unlock(&sign->quadrantClaimLock);
	lock(&sign->quadrantClaimLock);
	
	goThroughStopSign(car, &sign->base);
	unclaimQuadrants(car->index);
	// new quadrants have been freed up. wake up all car threads and tell them to re-check if they can claimQuadrants
	broadcastMultipleLanes(sign->laneCVs, QUADRANT_COUNT);
	
	unlock(&sign->quadrantClaimLock);	
	exitIntersection(car, lane);
	//as we lock the lane until the car exits, we ensure order of cars who exit
	unlock(&sign->laneMutexes[laneNum]);

	free(quadrantsNeeded);
}