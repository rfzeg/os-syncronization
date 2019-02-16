#pragma once
/**
* CSC369 Assignment 2
*
* This is the header file for your safe stop sign submission code.
*/
#include "car.h"
#include "stopSign.h"
/**
* @brief Structure that you can modify as part of your solution to implement
* proper synchronization for the stop sign intersection.
*
* This is basically a wrapper around StopSign, since you are not allowed to 
* modify or directly access members of StopSign.
*/
typedef struct _SafeStopSign {
	/**
	* @brief The underlying stop sign.
	*
	* You are not allowed to modify the underlying stop sign or directly
	* access its members. All interactions must be done through the functions
	* you have been provided.
	*/
	StopSign base;
	// TODO: Add any members you need for synchronization here.
	pthread_mutex_t quadrantClaimLock; // only one thread should be able to make reservations at a time
    // An array to hold mutexes corresponding to each lane
	pthread_mutex_t laneMutexArr[QUADRANT_COUNT];
    // An array to hold condition variables corresponding to each lane
	pthread_cond_t laneCondVarArr[QUADRANT_COUNT];
} SafeStopSign;
/**
* @brief Claim quadrants that this car intends to use for its action
* @return 1 if quadrants were claimed, 0 otherwise.
*
* @param quadrants the quadrants the car wants to reserve
* @param numClaims the number of quadrants being claimed (quadrants.length)
* @param carIndex the index of the car making the claims
*/
int claimQuadrants(int *quadrants, int numClaims, int carIndex);
/**
 * Unclaim the quadrants currently reserved by the car
 *
 * @param carIndex the index of the car to remove reservations from
 */
void unclaimQuadrants(int carIndex);
/**
 * Wake up cars that are waiting so they can check
 * @param sign
 */
void broadcastAllLanes(SafeStopSign* sign);
/**
* @brief Initializes the safe stop sign.
*
* @param sign pointer to the instance of SafeStopSign to be initialized.
* @param carCount number of cars in the simulation.
*/
void initSafeStopSign(SafeStopSign* sign, int carCount);
/**
* @brief Destroys the safe stop sign.
*
* @param sign pointer to the instance of SafeStopSign to be freed
*/
void destroySafeStopSign(SafeStopSign* sign);
/**
* @brief Runs a car-thread in a stop-sign scenario.
*
* @param car pointer to the car.
* @param sign pointer to the stop sign intersection.
*/
void runStopSignCar(Car* car, SafeStopSign* sign);
