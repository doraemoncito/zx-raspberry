# To build this project for Raspberry Pi 1, run the following commands in the console:
#
#   mkdir cmake-build-raspi
#   cd cmake-build-raspi
#   cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arm-none-eabi.cmake ..
#
# NOTE: by default, CMake generates standard UNIX makefiles.  To generate an XCode  project on MacOS, invoke CMake
# from the build directory as shown below:
#
#   cmake -G Xcode ..
#

cmake_minimum_required(VERSION 3.16.3)
project(zx-raspberry-project LANGUAGES C CXX)

# Do NOT generate verbose makefiles
# [Using CMake with GNU Make: How can I see the exact commands?](https://stackoverflow.com/questions/2670121/using-cmake-with-gnu-make-how-can-i-see-the-exact-commands)
set(MAKEFLAGS ADD --no-print-directory)
set(CMAKE_RULE_MESSAGES OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Require out-of-source builds
file(TO_CMAKE_PATH "${CMAKE_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif ()

#set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

# FIXME: these do not appear to work
set(CMAKE_ASM_OUTPUT_EXTENSION .o)
set(CMAKE_C_OUTPUT_EXTENSION .o)
set(CMAKE_CXX_OUTPUT_EXTENSION .o)

# Replace the file extension (rather than append .obj to it) when generating object files
set(CMAKE_ASM_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

set(AARCH 32)
set(RASPPI 4)
set(PREFIX32 arm-none-eabi-)
set(PREFIX64 aarch64-none-elf-)

# set this to "softfp" if you want to link specific libraries
set(FLOAT_ABI hard)

set(CPP ${CMAKE_CXX_COMPILER})

if (${AARCH} MATCHES 32)

    message(NOTICE "[INFO] Architecture: 32 bits")

    if (${RASPPI} MATCHES 1)
        message(NOTICE "[INFO] Device: Raspberry Pi 1 (32 bits)")
        set(ARCH "-DAARCH=32 -mcpu=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=${FLOAT_ABI}")
        set(TARGET kernel)
    elseif (${RASPPI} MATCHES 2)
        message(NOTICE "[INFO] Device: Raspberry Pi 2 (32 bits)")
        set(ARCH "-DAARCH=32 -mcpu=cortex-a7 -marm -mfpu=neon-vfpv4 -mfloat-abi=${FLOAT_ABI}")
        set(TARGET kernel7)
    elseif (${RASPPI} MATCHES 3)
        message(NOTICE "[INFO] Device: Raspberry Pi 3 (32 bits)")
        set(ARCH "-DAARCH=32 -mcpu=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=${FLOAT_ABI}")
        set(TARGET kernel8-32)
    elseif (${RASPPI} MATCHES 4)
        message(NOTICE "[INFO] Device: Raspberry Pi 4 (32 bits)")
        set(ARCH "-DAARCH=32 -mcpu=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=${FLOAT_ABI}")
        set(TARGET kernel7l)
    else ()
        message(FATAL_ERROR "error RASPPI must be set to 1, 2, 3 or 4)")
    endif ()

    set(PREFIX ${PREFIX32})
    set(LOADADDR 0x8000)

elseif (${AARCH} MATCHES 64)

    message(NOTICE "[INFO] Architecture: 64 bits")

    if (${RASPPI} MATCHES 3)
        message(NOTICE "[INFO] Device: Raspberry Pi 3 (64 bits)")
        set(ARCH "-DAARCH=64 -mcpu=cortex-a53 -mlittle-endian -mcmodel=small")
        set(TARGET kernel8)
    elseif (${RASPPI} MATCHES 4)
        message(NOTICE "[INFO] Device: Raspberry Pi 4 (64 bits)")
        set(ARCH "-DAARCH=64 -mcpu=cortex-a72 -mlittle-endian -mcmodel=small")
        set(TARGET kernel8-rpi4)
    else ()
        message(FATAL_ERROR "error RASPPI must be set to 3 or 4")
    endif ()

    set(PREFIX ${PREFIX64})
    set(LOADADDR 0x80000)

else ()
    message(FATAL_ERROR "error AARCH must be set to 32 or 64)")
endif ()

# Select the version of the Raspberry Pi board we are building for.  This is required because circle uses aliases for
# some classes and the definition of the variable RASPPI is required at build time.  Ideally this would be baked into
# Circle so we wouldn't have to do this.
add_definitions(-DRASPPI=${RASPPI})
# For newer versions of CMake
#add_compile_definitions(RASPPI=${RASPPI})

# For the bare metal Raspberry Pi system we can only build the static library
if (${CMAKE_SYSTEM_NAME} MATCHES "Generic")
    set(Z80CPP_STATIC_ONLY true)
endif ()

# https://stackoverflow.com/questions/5096881/does-set-target-properties-in-cmake-override-cmake-cxx-flags
# https://gitlab.kitware.com/cmake/cmake/issues/17991
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS_FOR_TARGET} -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS_FOR_TARGET} -O0 -Wall -Wextra")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS_FOR_TARGET} -O0 -Wall -Wextra -pedantic")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS_FOR_TARGET} -O0 -Wall -Wextra -pedantic")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS_FOR_TARGET} -O0 -Wall -Wextra -pedantic -Werror")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS_FOR_TARGET} -O0 -Wall -Wextra -pedantic -Werror")
#set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS ${BUILD_FLAGS})

# Set the rpath policy and C++ version for MacOS builds
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    # set the RPATH policy
    set(CMAKE_MACOSX_RPATH 1)

    # specify the C++ standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED True)

endif()

set (API_REVISION 1)
set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (RELEASE_TYPE "")
set (VERSION_STR "${API_REVISION}.${VERSION_MAJOR}.${VERSION_MINOR}${RELEASE_TYPE}")

if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "-Wall -O0 -std=c++17 ${CFLAGS_FOR_TARGET} -Wall -Wextra")
#    set (CMAKE_CXX_FLAGS "-Wall -O0 -std=c++14 ${CFLAGS_FOR_TARGET} -Wall -Wextra -pedantic")
#    set (CMAKE_CXX_FLAGS "-Wall -O0 -std=c++14 ${CFLAGS_FOR_TARGET} -Wall -Wextra -pedantic -Werror")
#    set (CMAKE_CXX_FLAGS "-Wall -O3 -std=c++14 ${CFLAGS_FOR_TARGET}")
endif ()

#include_directories(BEFORE . include)
#
#add_subdirectory(examples)

#set (z80cpp_sources src/z80.cpp include/z80.h include/z80operations.h )
#add_library (z80cpp-static STATIC ${z80cpp_sources})
#set_target_properties (z80cpp-static PROPERTIES OUTPUT_NAME z80cpp)
#if (NOT DEFINED Z80CPP_STATIC_ONLY)
#    add_library (z80cpp SHARED ${z80cpp_sources})
#    # Affects Win32 only: avoid dynamic/static *.lib files naming conflict
#    set_target_properties (z80cpp-static PROPERTIES PREFIX "lib")
#endif ()
#
#if (NOT DEFINED Z80CPP_STATIC_ONLY)
#    set_target_properties(z80cpp
#        PROPERTIES VERSION ${VERSION_STR} SOVERSION ${API_REVISION}
#    )
#endif ()
#
#set_target_properties(z80cpp-static
#    PROPERTIES VERSION ${VERSION_STR} SOVERSION ${API_REVISION}
#)
#
#if ("${CMAKE_CPP_IMPLICIT_LINK_DIRECTORIES}" MATCHES "lib64")
#    set (LIB_DIR "lib64")
#else ()
#    set (LIB_DIR "lib")
#endif ()
#if (NOT DEFINED Z80CPP_STATIC_ONLY)
#    install (TARGETS z80cpp LIBRARY DESTINATION ${LIB_DIR} ARCHIVE DESTINATION ${LIB_DIR})
#endif ()

# if the target is bare metal ARM (e.g. Raspberry Pi) ...
if (${CMAKE_SYSTEM_NAME} MATCHES "Generic")

    # Stop CMake from linking against 'arm-none-eabi/lib/libstdc++.a'
    set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
    set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")

    # FIXME: make this relative
    set(INCLUDE_PREFIX "/home/vagrant/zx-raspberry/lib")
    set(NEWLIB_ARCH "arm-none-circle")
    # For 64 bit support replace with this architecture prefix
    # set( NEWLIB_ARCH "aarch64-none-circle" )
    set(NEWLIBDIR "${INCLUDE_PREFIX}/circle-stdlib/install/${NEWLIB_ARCH}")
    set(CIRCLEHOME "${INCLUDE_PREFIX}/circle-stdlib/libs/circle")

    include_directories(
            BEFORE
            ${NEWLIBDIR}/include
            ${CIRCLEHOME}/include
    )

    # Locate the standard C++ library
    execute_process(
            COMMAND bash "-c" "${CPP} ${ARCH} -print-file-name=libstdc++.a"
            OUTPUT_VARIABLE LIBSTDCPP
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (LIBSTDCPP)
        message(NOTICE "[INFO] Adding the C++ standard library: '${LIBSTDCPP}'")
        list(APPEND EXTRA_LIBS ${LIBSTDCPP})
    else()
        message(FATAL_ERROR "Unable to find the C++ standard library: 'libstdc++.a'")
    endif ()

    # Locate the GCC library
    execute_process(
            COMMAND bash "-c" "${CPP} ${ARCH} -print-file-name=libgcc.a"
            OUTPUT_VARIABLE LIBGCC
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (LIBGCC)
        message(NOTICE "[INFO] Adding GCC standard library: '${LIBGCC}'")
        list(APPEND EXTRA_LIBS ${LIBGCC})
    else()
        message(FATAL_ERROR "Unable to find GCC standard library: 'libgcc.a'")
    endif ()


    # WARNING: The order of the libraries in this statement MATTERS and the linker may not be able to resolve some
    # symbols if the order is changed.  DO NOT change the order of the libraries in this statement unless you know
    # what you are doing.
    set(
            LIBS
            "${NEWLIBDIR}/lib/libcirclenewlib.a"
            "${NEWLIBDIR}/lib/libc.a"
            "${NEWLIBDIR}/lib/libg.a"
            "${NEWLIBDIR}/lib/libm.a"
            "${NEWLIBDIR}/lib/libnosys.a"
            "${CIRCLEHOME}/addon/SDCard/libsdcard.a"
#            "${CIRCLEHOME}/lib/bt/libbluetooth.a"
            "${CIRCLEHOME}/lib/fs/fat/libfatfs.a"
            "${CIRCLEHOME}/lib/fs/libfs.a"
            "${CIRCLEHOME}/lib/input/libinput.a"
            "${CIRCLEHOME}/lib/libcircle.a"
            "${CIRCLEHOME}/lib/net/libnet.a"
            "${CIRCLEHOME}/lib/sched/libsched.a"
            "${CIRCLEHOME}/lib/usb/libusb.a"
            ${EXTRA_LIBS}
    )

else (${CMAKE_SYSTEM_NAME} MATCHES "Generic")

    # Add the Circle compatibility library to the include search path
    include_directories(BEFORE SYSTEM src/compatibility)

endif (${CMAKE_SYSTEM_NAME} MATCHES "Generic")

# List include directories for debugging
# https://stackoverflow.com/questions/6902149/listing-include-directories-in-cmake
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach (dir ${dirs})
    message(STATUS "Include directory: '${dir}'")
endforeach ()

# Enable running the test simulator as part the `make test` target
#set( TEST_SOURCES
#    example/z80sim.cpp
#    example/z80sim.h
#)

#add_executable( z80sim ${TEST_SOURCES} )
#target_link_libraries( z80sim z80cpp-static ${CIRCLE_LIBS} )
#
#if( ${CMAKE_SYSTEM_NAME} MATCHES "Generic" )
#    # Stop CMake from linking against 'arm-none-eabi/lib/libstdc++.a'
#    set_target_properties( z80sim PROPERTIES LINKER_LANGUAGE CXX )
#endif()
#
#configure_file( example/zexall.bin zexall.bin COPYONLY )
#
#enable_testing( ) 
#add_test( NAME z80sim COMMAND z80sim )

#install( TARGETS z80cpp-static LIBRARY DESTINATION ${LIB_DIR} ARCHIVE DESTINATION ${LIB_DIR} )
#install( DIRECTORY include/ DESTINATION include/z80cpp PATTERN "*.h" )

function(generate_kernel_image PROJECT_NAME OUTPUT_NAME)

    #message(STATUS "generate_kernel_image arguments: ${PROJECT_NAME}, followed by ${ARGN}")
    get_filename_component(OUTPUT_NAME_WE ${OUTPUT_NAME} NAME_WE)

   add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND ${CROSS_COMPILE}objdump -d ${PROJECT_NAME} | ${CROSS_COMPILE}c++filt > ${OUTPUT_NAME_WE}.lst
            COMMENT "  DUMP   ${OUTPUT_NAME_WE}.lst"
            VERBATIM
    )

    # Strip the debugging information from the resulting ELF application file
    add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND ${CROSS_COMPILE}strip ${OUTPUT_NAME}
            COMMENT "  STRIP  ${OUTPUT_NAME}"
            VERBATIM
    )

    # Copy and translate the application file into a binary kernel image
    add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND ${CROSS_COMPILE}objcopy ${OUTPUT_NAME} -O binary ${OUTPUT_NAME_WE}.img
            COMMENT "  COPY   ${OUTPUT_NAME_WE}.img"
            VERBATIM
    )

    # Print the byte count for the file
    add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND wc -c < ${OUTPUT_NAME_WE}.img
            COMMENT "  WC     ${OUTPUT_NAME_WE}.img => "
            VERBATIM
    )

    # Compress the kernel image
    add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND gzip -c ${OUTPUT_NAME_WE}.img > ${OUTPUT_NAME_WE}.img.gz
            COMMENT "  GZIP   ${OUTPUT_NAME_WE}.img => ${OUTPUT_NAME_WE}.img.gz"
            VERBATIM
    )

    # Compress the ELF image
    add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND gzip -c ${PROJECT_NAME} > ${PROJECT_NAME}.gz
            COMMENT "  GZIP   ${PROJECT_NAME} => ${PROJECT_NAME}.gz"
            VERBATIM
    )

    # Wrap the uncompressed kernel image in an U-Boot legacy header image file so that it can be sent over to U-Boot via
    # TFTP e.g. `tftp ${kernel_addr_r} ${bootfile}` and run with bootm e.g. `bootm ${kernel_addr_r} - ${fdt_addr}`.
    add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND mkimage -n "ZX Raspberry ${VERSION_STR}" -A arm -O linux -T kernel -C none -a ${LOAD_ADDR} -e ${ENTRY_POINT} -d ${OUTPUT_NAME_WE}.img ${OUTPUT_NAME_WE}.uimg && mv ${OUTPUT_NAME_WE}.uimg "${CMAKE_BINARY_DIR}/"
            # NOTE: to send a compressed kernel image we need to pass the `-C gzip -d ${OUTPUT_NAME_WE}.img.gz` arguments to mkimage
            # COMMAND mkimage -n "ZX Raspberry ${VERSION_STR}" -A arm -O linux -T kernel -C gzip -a ${LOAD_ADDR} -e ${ENTRY_POINT} -d ${OUTPUT_NAME_WE}.img.gz ${OUTPUT_NAME_WE}.uimg && mv ${OUTPUT_NAME_WE}.uimg "${CMAKE_BINARY_DIR}/"
            COMMENT "  U-BOOT ${OUTPUT_NAME_WE}.img => ${OUTPUT_NAME_WE}.uimg"
            VERBATIM
    )

endfunction()

add_subdirectory( src )
