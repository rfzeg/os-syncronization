/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe traffic light 
* submission code.
*/
#include "safeTrafficLight.h"
#include "syncUtils.h"

void initSafeTrafficLight(SafeTrafficLight* light, int horizontal, int vertical) {
	initTrafficLight(&light->base, horizontal, vertical);

	int i;
	for (i=0;i<TRAFFIC_LIGHT_LANE_COUNT;i++){
		initMutex(&light->laneQueueLocks[i]);
		initConditionVariable(&light->laneQueueCVs[i]);
        light->laneQueues[i] = initIntQueue();
	}
	for(i = 0; i < 2;i++) {
		initMutex(&light->collisionLocks[i]);
		initConditionVariable(&light->collisionCVs[i]);

	}
	initMutex(&light->trafficLightLock);

}

void destroySafeTrafficLight(SafeTrafficLight* light) {
	destroyTrafficLight(&light->base);

	int i;
	for (i=0;i < TRAFFIC_LIGHT_LANE_COUNT;i++){
		destroyMutex(&light->laneQueueLocks[i]);
		destroyConditionVariable(&light->laneQueueCVs[i]);
		freeQueue(light->laneQueues[i]);
	}
	for(i=0; i < 2; i++) {
		destroyMutex(&light->collisionLocks[i]);
		destroyConditionVariable(&light->collisionCVs[i]);
	}
	destroyMutex(&light->trafficLightLock);
}

void runTrafficLightCar(Car* car, SafeTrafficLight* light) {
	int laneIndex = getLaneIndexLight(car);

	lock(&light->laneQueueLocks[laneIndex]);

	EntryLane* lane = getLaneLight(car, &light->base);
	enterLane(car, lane);
	enqueue(light->laneQueues[laneIndex], car->index);

	// Enter and act are separate calls because a car turning left can first
	// enter the intersection before it needs to check for oncoming traffic.
	lock(&light->trafficLightLock);
	while (!canEnterIntersection(car, light)){
		cvWait(&light->laneQueueCVs[laneIndex], &light->trafficLightLock);
	}
	enterTrafficLight(car, &light->base);

	broadcastMultipleLanes(light->laneQueueCVs, TRAFFIC_LIGHT_LANE_COUNT);

	int collisionLockIndex = car->position % 2;

	if (car->action == LEFT_TURN){
			lock(&light->collisionLocks[collisionLockIndex]);
			CarPosition opposite = getOppositePosition(car->position);

			//check whether any cars going straight in the opposite direction
			while (getStraightCount(&light->base, (int) opposite) > 0){
				cvWait(&light->collisionCVs[collisionLockIndex], &light->collisionLocks[collisionLockIndex]);
			}
			broadcastMultipleLanes(light->collisionCVs, 2);

            unlock(&light->collisionLocks[collisionLockIndex]);
	}

	actTrafficLight(car, &light->base, NULL, NULL, NULL);
    unlock(&light->trafficLightLock);

	//ensure car who entered lane first also exits first -- maintains queue order
	while(light->laneQueues[laneIndex]->size > 0 && car->index != light->laneQueues[laneIndex]->head->val){
		cvWait(&light->laneQueueCVs[laneIndex], &light->laneQueueLocks[laneIndex]);
	}
	exitIntersection(car, lane);
	dequeue(light->laneQueues[laneIndex]);

	broadcastMultipleLanes(light->laneQueueCVs, TRAFFIC_LIGHT_LANE_COUNT);

	unlock(&light->laneQueueLocks[laneIndex]);
}

int canEnterIntersection(Car* car, SafeTrafficLight* light) {
	if (car->position == 0 || car->position == 2){
		//Check East-West
		if (getLightState(&light->base) != 1){
			return 0;
		}
		return 1;
	}
	else{
		//Check North-South
		if (getLightState(&light->base) != 0){
			return 0;
		}
		return 1;
	}
}