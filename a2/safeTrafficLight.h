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
	pthread_mutex_t eastStraightLock;
	pthread_mutex_t eastLeftLock;
	pthread_mutex_t eastRightLock;

	pthread_mutex_t southStraightLock;
	pthread_mutex_t southLeftLock;
	pthread_mutex_t southRightLock;

	pthread_mutex_t westStraightLock;
	pthread_mutex_t westLeftLock;
	pthread_mutex_t westRightLock;

	pthread_mutex_t northStraightLock;
	pthread_mutex_t northLeftLock;
	pthread_mutex_t northRightLock;

	pthread_cond_t eastStraightCV;
	pthread_cond_t eastLeftCV;
	pthread_cond_t eastRightCV;

	pthread_cond_t southStraightCV;
	pthread_cond_t southLeftCV;
	pthread_cond_t southRightCV;

	pthread_cond_t westStraightCV;
	pthread_cond_t westLeftCV;
	pthread_cond_t westRightCV;

	pthread_cond_t northStraightCV;
	pthread_cond_t northLeftCV;
	pthread_cond_t northRightCV;


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
