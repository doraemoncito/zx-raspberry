# Toolchain for Raspberry Pi 4B 64 bit
#
# Invoke CMake with the arguments bellow to build the project for Raspberry Pi in release mode:
#
#   cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm-aarch64-elf.cmake ..
#
# or like this in debug mode:
#
#   cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm-aarch64-elf.cmake ..
#
# the machine architecture of the compiler can be found by running:                                                                         
#                                                                                                                                           
#   aarch64-elf-g++ -dumpmachine
#
# which should return "aarch64-elf".
#

include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# set the toolchain prefix for cross compilation
set(CROSS_COMPILE aarch64-elf-)

set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_C_COMPILER_TARGET ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
set(CMAKE_CXX_COMPILER_TARGET ${CROSS_COMPILE}g++)
set(CMAKE_LINKER ${CROSS_COMPILE}ld)

# SET( LINK_FLAGS "--map --ro-base=0x0 --rw-base=0x0008000 --first='boot.o(RESET)' --datacompressor=off")
# set( CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" )
# set( CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" )
# set( CMAKE_C_LINK_EXECUTABLE ${toolchain_prefix}ld )
# set( CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" )

# https://stackoverflow.com/questions/43781207/how-to-cross-compile-with-cmake-arm-none-eabi-on-windows)
set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs" CACHE INTERNAL "")

# set( CMAKE_EXE_LINKER_FLAGS CACHE INTERNAL "" )
# unset( CMAKE_EXE_LINKER_FLAGS CACHE )
# set( CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "" FORCE )

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
