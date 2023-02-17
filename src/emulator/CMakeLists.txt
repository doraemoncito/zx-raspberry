cmake_minimum_required(VERSION 3.16.3)
project(zxraspberry LANGUAGES C CXX)

cmake_minimum_required(VERSION 3.16.3)

project(
        zxraspberry
        VERSION 1.0.0
        DESCRIPTION "Bare metal ZX Spectrum emulator for Raspberry Pi"
        HOMEPAGE_URL "https://github.com/doraemoncito/zx-raspberry"
        LANGUAGES C CXX
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


# Required to compile Qt code on macOS
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


# Add OpenMP support if the platform supports it
#if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Xpreprocessor -fopenmp")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fopenmp")
# https://cliutils.gitlab.io/modern-cmake/chapters/packages/OpenMP.html
#    find_package(OpenMP)
#    if(OpenMP_CXX_FOUND)
#        target_link_libraries(MyTarget PUBLIC OpenMP::OpenMP_CXX)
#    endif()
#endif()

include_directories(BEFORE ../../include)

add_executable(
        ${PROJECT_NAME}
        common/keyboard.h
        common/zxdisplay.h
        common/zxdisplay.cpp
        common/Z80emu.h
        common/Z80emu.cpp
        common/clock.cpp
        common/clock.h
        common/z80.cpp
        common/zx48k_rom.cpp
        common/gui/zxpoint.h
        common/gui/zxrect.h
        common/gui/zxpoint.cpp
        common/gui/zxrect.cpp
        common/gui/zxdialog.cpp
        common/gui/zxdialog.h
        common/gui/zxlabel.cpp
        common/gui/zxlabel.h
        common/gui/zxview.cpp
        common/gui/zxview.h
        common/gui/zxgroup.cpp
        common/gui/zxgroup.h
        common/hardware/zxhardwaremodel.cpp
        common/hardware/zxhardwaremodel.h
        common/hardware/zxhardwaremodel48k.cpp
        common/hardware/zxhardwaremodel48k.h
        )

include_directories(BEFORE include)

# Use Qt and our Circle compatibility library in MacOS builds
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

    target_sources(
            ${PROJECT_NAME} PRIVATE
            macos/main.cpp
            macos/zxemulatorscreen.cpp
            macos/zxemulatorscreen.h
            macos/zxemulatorwindow.cpp
            macos/zxemulatorwindow.h
            macos/zxkeyboard.cpp
            macos/zxkeyboard.h
            # TODO: move this to its own compatibility library
            ../compatibility/circle/logger.cpp
            ../compatibility/circle/util.cpp
    )

    include_directories(BEFORE SYSTEM ../compatibility)

    target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::Widgets)

    # Set the output directory so that the executable goes to the to level build directory
    set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/"
    )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# https://stackoverflow.com/questions/31343813/displaying-cmake-variables
macro(print_all_variables)
    message(STATUS "--------------------------- BEGIN CMAKE VARIABLES ----------------------------")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "---------------------------- END CMAKE VARIABLES -----------------------------")
endmacro()

# https://cmake.org/pipermail/cmake/2012-March/049620.html
# https://stackoverflow.com/questions/8591762/ifdef-debug-with-cmake-independent-from-platform
IF(${CMAKE_BUILD_TYPE} MATCHES Debug)
    # Use pre-preprocessor in code like this to detect debug mode: #ifndef NDEBUG
    message("debug mode enabled")
    print_all_variables()

    # define the DEBUG preprocessor constant (TODO: is this standard C++? CMake itself seems to define NDEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
ENDIF(${CMAKE_BUILD_TYPE} MATCHES Debug)

# if the target is bare metal ARM (e.g. Raspberry Pi) ...
if (${CMAKE_SYSTEM_NAME} MATCHES "Generic")

    include_directories(
            common
    )

    target_include_directories(
            ${PROJECT_NAME} PRIVATE
            raspi
    )

    #    set( EXTRALIBS
    #        z80cpp-static
    #    )

    target_sources(
            ${PROJECT_NAME} PRIVATE
            raspi/main.cpp
            raspi/kernel.cpp
            raspi/zxkeyboard.cpp
            raspi/zxkeyboard.h
            raspi/zxgamepad.cpp
            raspi/zxgamepad.h
            raspi/zxula.cpp
            raspi/zxula.h
            raspi/gamepad/KempstonGamePadAdapter.cpp
            raspi/gamepad/KempstonGamePadAdapter.h
            raspi/gamepad/CursorJoystickAdapter.cpp
            raspi/gamepad/CursorJoystickAdapter.h
            raspi/gamepad/JoystickAdapter.cpp
            raspi/gamepad/JoystickAdapter.h
            raspi/gamepad/FullerGamePadAdapter.cpp
            raspi/gamepad/FullerGamePadAdapter.h
            raspi/gamepad/Sinclair1GamePadAdapter.cpp
            raspi/gamepad/Sinclair1GamePadAdapter.h
            raspi/gamepad/Sinclair2GamePadAdapter.cpp
            raspi/gamepad/Sinclair2GamePadAdapter.h
            raspi/gamepad/KeyboardGamePadAdapter.cpp
            raspi/gamepad/KeyboardGamePadAdapter.h
            raspi/gamepad/NullGamePadAdapter.cpp
            raspi/gamepad/NullGamePadAdapter.h
    )

    set(KERNEL_IMG kernel7l.img)
    set(LOAD_ADDR 0x00008000)
    set(ENTRY_POINT 0x00008000)

    # google search "cmake target_objects example"
# https://cmake.org/cmake/help/v3.0/command/add_library.html
#    add_library(
#            ${PROJECT_NAME}_OBJECTS
#            OBJECT
#            raspi/main.cpp
#            raspi/kernel.cpp
#    )

    set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> -Wl,-Map=${TARGET}.map ${LDFLAGS} -T ${CIRCLEHOME}/circle.ld ${CRTBEGIN} <OBJECTS> -nostdlib -Wl,--start-group <LINK_LIBRARIES> -Wl,--end-group ${CRTEND} -o <TARGET>")
    set(ENTRY_POINT 0x00008000)

    # https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html
    # After printing the whole g++ command like by invoking it with the "-v" argument we can see that g++
    # adds some additional C runtime libraries to the ld command line, namely: crtbegin, crti and crt0.
    # One of those files references the 'bss_start' and 'bss_end' symbols and since we don't need any
    # of those files for linking we can remove them from the linker command line by passing the "-nostdlib"
    # argument to g++.  For more details please see this link:
    # https://stackoverflow.com/questions/57357726/linking-failed-when-use-indirectly-the-gnu-linker
    #
    # Build the application
#    target_link_libraries(
#            ${PROJECT_NAME}.elf PUBLIC
#            "-nostdlib"
#            "-T ${CIRCLEHOME}/circle.ld"
##            "$<TARGET_OBJECTS:${PROJECT_NAME}_OBJECTS>"
#            "-Wl,--section-start=.init=${ENTRY_POINT}"
#            "-Wl,--start-group"
#            ${LIBS}
#            ${EXTRALIBS}
#            "-Wl,--end-group"
#    )

    # post cmake 3.13 we ca use this:
    #target_link_options( ${PROJECT_NAME} PUBLIC "-T $(CIRCLEHOME)/circle.ld" )

    get_filename_component(OUTPUT_NAME ${PROJECT_NAME} NAME_WE)
    set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES
            OUTPUT_NAME ${OUTPUT_NAME}
            SUFFIX ""
    )

#    add_executable(${PROJECT_NAME}.elf main.cpp kernel.cpp)
#    set(LIBS circle)rm zx
    target_link_libraries(${PROJECT_NAME} ${LIBS} ${EXTRA_LIBS})
    generate_kernel_image(${PROJECT_NAME} ${PROJECT_NAME})

#    generate_kernel_image(${PROJECT_NAME} ${OUTPUT_NAME}.elf)

endif ()

install(
        TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION "${CMAKE_BINARY_DIR}"
        BUNDLE DESTINATION "${CMAKE_BINARY_DIR}"
        LIBRARY DESTINATION "${CMAKE_BINARY_DIR}"
)
