cmake_minimum_required(VERSION 3.10.2)
project(zxraspberry LANGUAGES CXX)

# Bare metal ARM (e.g. Raspberry Pi) builds only...
if (${CMAKE_SYSTEM_NAME} MATCHES "Generic")
    add_subdirectory(utilities/screentext)
endif ()

# MacOS builds only...
if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    add_subdirectory(zxtext)
endif()

# Both platforms, i.e. MacOS and bare metal Raspberry Pi
add_subdirectory(zxscreen)
add_subdirectory(zxgui)
