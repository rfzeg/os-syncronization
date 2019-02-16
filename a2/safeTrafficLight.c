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
        light->intQueueArr[i] = initIntQueue();
	}
	for(int j=0;j<2;j++) {
		initMutex(&light->collisionLocks[j]);
		initConditionVariable(&light->collisionCVs[j]);

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
		destroyConditionVariable(&light->collisionCVs[j]);
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
	printf("\nPRINT A BEFORE ENTERING INTERSECTION | CAR: %d, doing action %d from position %d\n", car->index, car->action, car->position);
	while (!canEnterIntersection(car, light)){
		printf("\nPRINT A.5 WAITING BC COULDNT ENTER INTERSECTION | CAR: %d, doing action %d from position %d\n", car->index, car->action, car->position);
		pthread_cond_wait(&light->cvArr[laneIndex], &light->trafficLightLock);
	}
	printf("\nPRINT A AFTER: GOT GO AHEAD TO ENTER INTERSECTION. CALLING enterTrafficLight | CAR: %d, doing action %d from position %d\n", car->index, car->action, car->position);
	enterTrafficLight(car, &light->base);
	//Broadcast all lanes
	int i;
	for (i=0;i<TRAFFIC_LIGHT_LANE_COUNT;i++){
		pthread_cond_broadcast(&light->cvArr[i]);
	}
	int collisionLockIndex = car->position % 2;
	printf("COLLISION LOCK INDEX FOR CAR %d is %d\n", car->index, collisionLockIndex);
	switch (car->action) {
		case 0: // straight
//			unlock(&light->trafficLightLock);
			// get the collision lock
			lock(&light->collisionLocks[collisionLockIndex]);

			actTrafficLight(car, &light->base, NULL, NULL, NULL);

			unlock(&light->collisionLocks[collisionLockIndex]);

			break;
		case 1: // right turns
//			unlock(&light->trafficLightLock);
			actTrafficLight(car, &light->base, NULL, NULL, NULL);
			break; // car going right. nothing to do
		case 2: // left turn s
//			unlock(&light->trafficLightLock);

			lock(&light->straightLock);
			lock(&light->collisionLocks[collisionLockIndex]);
			CarPosition opposite = getOppositePosition(car->position);
			printf("\nPRINT B: WANT TO TURN LEFT. ABOUT TO GET STRAIGHT COUNT | CAR: %d, doing action %d from position %d\n", car->index, car->action, car->position);
			while (getStraightCount(&light->base, (int) opposite) > 0){
				printf("\nPRINT B.5: WAITING BC STRAIGHT COUNT > 0. SO CANT MAKE LEFT TURN NOW | CAR: %d, doing action %d from position %d\n", car->index, car->action, car->position);
				pthread_cond_wait(&light->collisionCVs[collisionLockIndex], &light->collisionLocks[collisionLockIndex]);
			}
			printf("\nPRINT B AFTER: STRAIGHTCOUNT==0 SO ITS OK TO MAKE LEFT TURN | CAR: %d, doing action %d from position %d\n", car->index, car->action, car->position);
			// after acting on traffic light, we broadcast to let them know were done
			actTrafficLight(car, &light->base, NULL, NULL, NULL);
            for (i = 0; i < 2; i++) {
                pthread_cond_broadcast(&light->collisionCVs[i]);
            }
            unlock(&light->collisionLocks[collisionLockIndex]);
			unlock(&light->straightLock);
            break;
		default:
			unlock(&light->trafficLightLock);
			break;
	}
    unlock(&light->trafficLightLock);
    printf("\nPRINT C| CAR: %d, doing action %d from position %d\n", car->index, car->action, car->position);

	while(light->intQueueArr[laneIndex]->size > 0 && car->index != light->intQueueArr[laneIndex]->head->val){
		printf("\nWPRINT C.5: CAR %d MUST WAIT SINCE IT CANT EXIT AS IT IS NOT AT THE HEAD OF THE QUEUE; CAR %d IS AT THE HEAD.\n", car->index, light->intQueueArr[laneIndex]->head->val);
		pthread_cond_wait(&light->cvArr[laneIndex], &light->lockArr[laneIndex]);
	}
	printf("\nPRINT C AFTER: THE CAR IS AT THE HEAD OF THE QUEUE. SO IT IS OK TO EXIT | CAR: %d, doing action %d from position %d\n", car->index, car->action, car->position);

	exitIntersection(car, lane);
	dequeue(light->intQueueArr[laneIndex]);
//	pthread_cond_broadcast(&light->cvArr[laneIndex]);

    for (i = 0; i < 2; i++) {
        pthread_cond_broadcast(&light->collisionCVs[i]);
    }

    //Broadcast all lanes
    for (i=0;i<TRAFFIC_LIGHT_LANE_COUNT;i++){
        pthread_cond_broadcast(&light->cvArr[i]);
    }

	unlock(&light->lockArr[laneIndex]);
}

int canEnterIntersection(Car* car, SafeTrafficLight* light) {
	if (car->position == 0 || car->position == 2){
		if (getLightState(&light->base) != 1){
			printf("\nCAR %d WANTS TO ENTER INTERSECTION. LIGHT IS RED. return false\n", car->index);
			return 0;
		}
		printf("\nCAR %d WANTS TO ENTER INTERSECTION. LIGHT IS GREEN. return true\n", car->index);
		return 1;
	}
	else{
		if (getLightState(&light->base) != 0){
			printf("\nCAR %d WANTS TO ENTER INTERSECTION. LIGHT IS RED. return FALSE\n", car->index);
			return 0;
		}
		printf("\nCAR %d WANTS TO ENTER INTERSECTION. LIGHT IS GREEN. return true\n", car->index);
		return 1;
	}
}