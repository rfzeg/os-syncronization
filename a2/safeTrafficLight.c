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

	// TODO: Add any initialization logic you need.
	int i;
	for (i=0;i<TRAFFIC_LIGHT_LANE_COUNT;i++){
		initMutex(&light->lockArr[i]);
		initConditionVariable(&light->cvArr[i]);
	}
	initMutex(&light->trafficLightLock);
	initConditionVariable(&light->trafficLightCV);

}

void destroySafeTrafficLight(SafeTrafficLight* light) {
	destroyTrafficLight(&light->base);

	// TODO: Add any logic you need to free data structures
	int i;
	for (i=0;i<TRAFFIC_LIGHT_LANE_COUNT;i++){
		destroyMutex(&light->lockArr[i]);
		destroyConditionVariable(&light->cvArr[i]);
	}
	destroyMutex(&light->trafficLightLock);
	destroyConditionVariable(&light->trafficLightCV);
}

void runTrafficLightCar(Car* car, SafeTrafficLight* light) {

	// TODO: Add your synchronization logic to this function.
	int laneIndex = getLaneIndexLight(car);
	lock(&light->lockArr[laneIndex]);

	EntryLane* lane = getLaneLight(car, &light->base);
	enterLane(car, lane);

	// Enter and act are separate calls because a car turning left can first
	// enter the intersection before it needs to check for oncoming traffic.
	lock(&light->trafficLightLock);
	while(getLightState == 2){
		pthread_cond_wait(&light->trafficLightCV, &light->trafficLightLock);
	}
	enterTrafficLight(car, &light->base);
	pthread_cond_broadcast(&light->trafficLightLock);
	unlock(&light->trafficLightLock);
	actTrafficLight(car, &light->base, NULL, NULL, NULL);

	exitIntersection(car, lane);

	unlock(&light->lockArr[laneIndex]);


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


