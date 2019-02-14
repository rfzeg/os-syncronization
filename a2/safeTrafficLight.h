#pragma once
/**
* CSC369 Assignment 2
*
* This is the header file for your safe traffic light submission code.
*/
#include "car.h"
#include "trafficLight.h"

/**
* @brief Structure that you can modify as part of your solution to implement
* proper synchronization for the traffic light intersection.
*
* This is basically a wrapper around TrafficLight, since you are not allowed to 
* modify or directly access members of TrafficLight.
*/
typedef struct _SafeTrafficLight {

	/**
	* @brief The underlying light.
	*
	* You are not allowed to modify the underlying traffic light or directly
	* access its members. All interactions must be done through the functions
	* you have been provided.
	*/
	TrafficLight base;
	// TODO: Add any members you need for synchronization here.
	pthread_mutex_t lockArr[TRAFFIC_LIGHT_LANE_COUNT];
	pthread_mutex_t cvArr[TRAFFIC_LIGHT_LANE_COUNT];

} SafeTrafficLight;

/**
* @brief Initializes the safe traffic light.
*
* @param light pointer to the instance of SafeTrafficLight to be initialized.
* @param eastWest total number of cars moving east-west.
* @param northSouth total number of cars moving north-south.
*/
void initSafeTrafficLight(SafeTrafficLight* light, int eastWest, int northSouth);

/**
* @brief Destroys the safe traffic light.
*
* @param light pointer to the instance of SafeTrafficLight to be destroyed.
*/
void destroySafeTrafficLight(SafeTrafficLight* light);

/**
* @brief Runs a car-thread in a traffic light scenario.
*
* @param car pointer to the car.
* @param light pointer to the traffic light intersection.
*/
void runTrafficLightCar(Car* car, SafeTrafficLight* light);

/**
* @brief Destroys a mutex and does error checking.
*
* @param mutex pointer to the mutex to initialize.
*/
void destroyMutex(pthread_mutex_t* mutex);

/**
* @brief Destroys a condition variable and does error checking.
*
* @param cond pointer to the condition variable to initialize.
*/
void destroyConditionVariable(pthread_cond_t* cond);

/**
* @brief Locks a mutex and does error checking.
*
* @param mutex pointer to the mutex to lock.
*/
void lock(pthread_mutex_t *mutex);
