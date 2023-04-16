set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CTEST_OUTPUT_ON_FAILURE ON)
set (OUTPUT_JUNIT ON)

add_executable (z80_tests Z80Test.cpp ../emulator/common/z80.cpp)
include_directories (${DOCTEST_HOME})

target_include_directories (z80_tests PRIVATE
        ../emulator/include
        ${DOCTEST_HOME}
)

#target_link_libraries (z80_tests z80cpp-static)
add_test (NAME z80_tests COMMAND z80_tests)
