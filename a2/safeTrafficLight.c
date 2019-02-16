/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe traffic light 
* submission code.
*/
#include "safeTrafficLight.h"
#include "syncUtils.h"

int rightOfWay[4] = {0,0,0,0}; // 0 , 1, 2, 3 = east, n, w, s

void initSafeTrafficLight(SafeTrafficLight* light, int horizontal, int vertical) {
	initTrafficLight(&light->base, horizontal, vertical);

	// TODO: Add any initialization logic you need.
	int i;
	for (i=0;i<TRAFFIC_LIGHT_LANE_COUNT;i++){
		initMutex(&light->lockArr[i]);
		initConditionVariable(&light->cvArr[i]);
        light->intQueueArr[i] = initIntQueue();
	}
	for(int j=0;j<2;j++) {
		initMutex(&light->collisionLocks[j]);
	}
	initMutex(&light->trafficLightLock);
	initMutex(&light->straightLock);



}

void destroySafeTrafficLight(SafeTrafficLight* light) {
	destroyTrafficLight(&light->base);

	// TODO: Add any logic you need to free data structures
	int i;
	for (i=0;i<TRAFFIC_LIGHT_LANE_COUNT;i++){
		destroyMutex(&light->lockArr[i]);
		destroyConditionVariable(&light->cvArr[i]);
		freeQueue(light->intQueueArr[i]);
	}
	for(int j=0;j<2;j++) {
		destroyMutex(&light->collisionLocks[j]);
	}
	destroyMutex(&light->trafficLightLock);
	destroyMutex(&light->straightLock);
}

void runTrafficLightCar(Car* car, SafeTrafficLight* light) {

	// TODO: Add your synchronization logic to this function.
	int laneIndex = getLaneIndexLight(car);
	lock(&light->lockArr[laneIndex]);

	EntryLane* lane = getLaneLight(car, &light->base);
	enterLane(car, lane);
	enqueue(light->intQueueArr[laneIndex], car->index);

	// Enter and act are separate calls because a car turning left can first
	// enter the intersection before it needs to check for oncoming traffic.
	lock(&light->trafficLightLock);

	while (!canEnterIntersection(car, light)){
		pthread_cond_wait(&light->cvArr[laneIndex], &light->trafficLightLock);
	}
	enterTrafficLight(car, &light->base);

	//Broadcast all lanes
	int i;
	for (i=0;i<TRAFFIC_LIGHT_LANE_COUNT;i++){
		pthread_cond_broadcast(&light->cvArr[i]);
	}
	int collisionLockIndex = car->position % 2;
	switch (car->action) {
		case 0: //
			lock(&light->straightLock);
			rightOfWay[car->position] = 1;
			unlock(&light->straightLock);

			unlock(&light->trafficLightLock);
			// get the collision lock
			lock(&light->collisionLocks[collisionLockIndex]);

			actTrafficLight(car, &light->base, NULL, NULL, NULL);

			unlock(&light->collisionLocks[collisionLockIndex]);

			exitIntersection(car, lane);
			dequeue(light->intQueueArr[laneIndex]);

			lock(&light->straightLock);
			rightOfWay[car->position] = 0;
			unlock(&light->straightLock);

			break;
		case 1: // right turns
			unlock(&light->trafficLightLock);
			// get the collision lock
			actTrafficLight(car, &light->base, NULL, NULL, NULL);

			exitIntersection(car, lane);
			dequeue(light->intQueueArr[laneIndex]);
			break; // car going right. nothing to do
		case 2: // left turn s
			unlock(&light->trafficLightLock);
			// get the collision lock. FOR HERE, WE MUST MAKE A CV FOR LEFT TURNS SO THEY DONT BUSY WAIT
			lock(&light->collisionLocks[collisionLockIndex]);

			actTrafficLight(car, &light->base, NULL, NULL, NULL);

			unlock(&light->collisionLocks[collisionLockIndex]);

			exitIntersection(car, lane);
			dequeue(light->intQueueArr[laneIndex]);
			break;
		default:break;
	}
	//TODO peek at head of laneQueue to make sure order is maintained for each lane


	unlock(&light->lockArr[laneIndex]);
}

int canEnterIntersection(Car* car, SafeTrafficLight* light) {
	if (car->position == 0 || car->position == 2){
		if (getLightState(&light->base) != 1){
			return 0;
		}
		return 1;
	}
	else{
		if (getLightState(&light->base) != 0){
			return 0;
		}
		return 1;
	}
}





// some helpers from trafficLight.c

// LightState getOppositeDirection(LightState mode) {
// 	assert(mode != RED);
// 	return (LightState)(1 - (int)mode);
// }

// LightState getLightState(TrafficLight* light) {
// 	return light->currentMode;
// }

// int getStraightCount(TrafficLight* light, int position) {
// 	return light->straightCounts[position];
// }


