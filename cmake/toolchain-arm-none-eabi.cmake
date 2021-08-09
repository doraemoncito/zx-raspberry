# Toolchain for the original Raspberry Pi 1, 2, 3, 4 32 bits
#
# Invoke CMake with the arguments below to build the project for Raspberry Pi in release mode:
#
#   cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm-none-eabi.cmake ..
#
# or like this for DEBUG mode:
#
#   cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm-none-eabi.cmake ..
#
# the machine architecture of the compiler can be found by running:                                                                         
#                                                                                                                                           
#   arm-none-eabi-g++ -dumpmachine
#
# which should return "arm-none-eabi".
#

include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# set the toolchain prefix for cross compilation
set(CROSS_COMPILE arm-none-eabi-)

set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_C_COMPILER_TARGET ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
set(CMAKE_CXX_COMPILER_TARGET ${CROSS_COMPILE}g++)
set(CMAKE_LINKER ${CROSS_COMPILE}ld)

# RASPBERRY PI 1
#set(CFLAGS_FOR_TARGET "-DAARCH=32 -march=armv6k -mtune=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard -Wno-parentheses")

# RASPBERRY PI 4
set(CFLAGS_FOR_TARGET "-DAARCH=32 -march=armv8-a -mtune=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -Wno-parentheses")
set(TARGET kernel7l)

# https://stackoverflow.com/questions/43781207/how-to-cross-compile-with-cmake-arm-none-eabi-on-windows)
set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs" CACHE INTERNAL "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
