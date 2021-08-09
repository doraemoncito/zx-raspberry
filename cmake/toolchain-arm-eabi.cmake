# Toolchain for the original Raspberry Pi (i.e. version 1)
# NOTE: This is an old toolchain and it is now deprecated
#
# Invoke CMake with the arguments bellow to build the project for Raspberry Pi:
#                                                                                                                                           
#   cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arm-eabi.cmake ..
#                                                                                                                                           
# the machine architecture of the compiler can be found by running:                                                                         
#                                                                                                                                           
#   arm-eabi-g++ -dumpmachine
#
# which should return "arm-eabi".
#

include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# set the toolchain prefix for cross compilation
set(CROSS_COMPILE arm-eabi-)

set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_C_COMPILER_TARGET ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
set(CMAKE_CXX_COMPILER_TARGET ${CROSS_COMPILE}g++)
set(CMAKE_LINKER ${CROSS_COMPILE}ld)
set(CFLAGS_FOR_TARGET "-DAARCH=32 -march=armv6k -mtune=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard -Wno-parentheses")

# https://stackoverflow.com/questions/43781207/how-to-cross-compile-with-cmake-arm-none-eabi-on-windows)
set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs" CACHE INTERNAL "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
