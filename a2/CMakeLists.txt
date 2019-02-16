cmake_minimum_required(VERSION 3.10)
project(a2 C)

set(CMAKE_C_STANDARD 99)

#add_executable(car car.c)
#add_executable(common common.c)
#add_executable(intersection intersection.c)
#add_executable(mutexAccessValidator mutexAccessValidator.c)
#add_executable(safeStopSign safeStopSign.c)
#add_executable(safeTrafficLight safeTrafficLight.c)
#add_executable(stopSign stopSign.c)
#add_executable(trafficLight trafficLight.c)
#add_executable(carsim carsim.c)
#add_executable(syncUtils syncUtils.c)
#add_executable(testing testing.c)
#
file(GLOB a2_SRC
        "*.h"
        "*.c"
        )
add_executable(carsim ${a2_SRC})
