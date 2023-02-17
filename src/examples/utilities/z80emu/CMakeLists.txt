cmake_minimum_required( VERSION 3.10.2 )
project(z80emu LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples")

include_directories(
    ../../emulator/include
)

add_executable(
        z80emu
        common/zexall.h
        common/z80emu.h
        common/z80emu.cpp
        ../../emulator/common/z80.cpp
)

# Use Qt and our Circle compatibility library in MacOS builds
if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )

    find_package( Qt6 COMPONENTS Widgets REQUIRED )

    target_sources(
            z80emu PRIVATE
            macos/main.cpp
            macos/screen.cpp
            macos/screen.h
            macos/window.cpp
            macos/window.h
            ../../emulator/common/z80.cpp
    )

    include_directories(BEFORE ../../compatibility)
    target_link_libraries( z80emu PUBLIC Qt6::Widgets )

endif()

if( ${CMAKE_SYSTEM_NAME} MATCHES "Generic" )

    # Stop CMake from linking against 'arm-none-eabi/lib/libstdc++.a'
    set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
    set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")

    # FIXME: make this relative
    set( INCLUDE_PREFIX "/home/vagrant/src/z80cpp/build" )
    set( NEWLIB_ARCH "arm-none-circle" )
#    set( NEWLIB_ARCH "aarch64-none-circle" )
    set( NEWLIBDIR "${INCLUDE_PREFIX}/../../circle-stdlib/install/${NEWLIB_ARCH}" )
    set( CIRCLEHOME "${INCLUDE_PREFIX}/../../circle-stdlib/libs/circle" )

    include_directories(
            BEFORE
            ${NEWLIBDIR}/include
            ${CIRCLEHOME}/include
    )

    include_directories(
            shared
    )

    target_include_directories(
            z80emu PRIVATE
            raspberrypi
    )

    set(
            LIBS
            "${NEWLIBDIR}/lib/libc.a"
            "${NEWLIBDIR}/lib/libcirclenewlib.a"
            "${NEWLIBDIR}/lib/libg.a"
            "${NEWLIBDIR}/lib/libm.a"
            "${NEWLIBDIR}/lib/libnosys.a"
            "${CIRCLEHOME}/addon/SDCard/libsdcard.a"
            "${CIRCLEHOME}/lib/bt/libbluetooth.a"
            "${CIRCLEHOME}/lib/fs/fat/libfatfs.a"
            "${CIRCLEHOME}/lib/fs/libfs.a"
            "${CIRCLEHOME}/lib/input/libinput.a"
            "${CIRCLEHOME}/lib/libcircle.a"
            "${CIRCLEHOME}/lib/net/libnet.a"
            "${CIRCLEHOME}/lib/sched/libsched.a"
            "${CIRCLEHOME}/lib/usb/libusb.a"
            "/usr/local/opt/gcc-arm-8.2-2019.01-x86_64-arm-eabi/bin/../lib/gcc/arm-eabi/8.2.1/../../../../arm-eabi/lib/arm/v5te/hard/libstdc++.a"
            "/usr/local/opt/gcc-arm-8.2-2019.01-x86_64-arm-eabi/bin/../lib/gcc/arm-eabi/8.2.1/arm/v5te/hard/libgcc.a"
#            "/usr/local/opt/gcc-arm-8.3-2019.03-x86_64-aarch64-elf/aarch64-elf/lib/libstdc++.a"
#            "/usr/local/opt/gcc-arm-8.3-2019.03-x86_64-aarch64-elf/lib/gcc/aarch64-elf/8.3.0/libgcc.a"
#            "/usr/local/opt/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/lib/arm/v5te/hard/libstdc++.a"
#            "/usr/local/opt/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/arm/v5te/hard/libgcc.a"
    )

    set(EXTRALIBS z80cpp-static)

    target_sources(
            z80emu PRIVATE
            raspberrypi/main.cpp
            raspberrypi/kernel.cpp
    )

    set(LOAD_ADDR 0x0000)
    set(ENTRY_POINT 0x8000)

    # https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html
    # After printing the whole g++ command like by invoking it with the "-v" argument we can see that g++
    # adds some additional C runtime libraries to the ld command line, namely: crtbegin, crti and crt0.
    # One of thoses files references the 'bss_start' and 'bss_end' symbols and since we don't need any
    # of thoses files for linking we can remove them from the linker command line by passing the "-nostdlib"
    # argument to g++.  For more details please see this link:
    # https://stackoverflow.com/questions/57357726/linking-failed-when-use-indirectly-the-gnu-linker
    #
    target_link_libraries(
            ${PROJECT_NAME} PUBLIC
            "-nostdlib"
            "-T ${CIRCLEHOME}/circle.ld"
            "-Wl,--section-start=.init=${ENTRY_POINT}"
            "-Wl,--start-group"
            ${LIBS}
            ${EXTRALIBS}
            "-Wl,--end-group"
    )

    # post cmake 3.13 we ca use this:
    #target_link_options( z80emu PUBLIC "-T $(CIRCLEHOME)/circle.ld" )

    add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND ${CROSS_COMPILE}objdump -d ${PROJECT_NAME} | ${CROSS_COMPILE}c++filt > kernel.lst
            COMMENT "  DUMP   kernel.lst"
            VERBATIM
    )

    add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND ${CROSS_COMPILE}objcopy ${PROJECT_NAME} -O binary kernel.img
            COMMENT "  COPY   kernel.img"
            VERBATIM
    )

    add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND wc -c < kernel.img
            COMMENT "  WC     kernel.img => "
            VERBATIM
    )

    # U-Boot requires us to wrap the ELF executable (not the binary image) in a special U-Boot image file
    # so that it can be sent over to U-Boot via TFTP.
    # IMPORTANT: 32 bit kernels in Das U-Boot are loaded at address 0x0000 and started at address 0x8000.
    add_custom_command(
            TARGET ${PROJECT_NAME}
            # Run after the kernel has been built
            POST_BUILD
            COMMAND mkimage -n "Sinclair ZX (Spectrum) Pi" -A arm -O linux -T kernel -C none -a 0x0000 -e 0x8000 -d ${PROJECT_NAME} /tmp/boot.scr.uimg && mv /tmp/boot.scr.uimg .
            COMMENT "  U-Boot boot.scr.uimg"
            VERBATIM
    )

endif()

install(
        TARGETS z80emu
        RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
        BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
        LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
