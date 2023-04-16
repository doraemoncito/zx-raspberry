//
// Created by José Hernández on 16/04/2023.
//

#ifndef Z80CPP_Z80TEST_CPP
#define Z80CPP_Z80TEST_CPP


#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "z80.h"

Z80 target(nullptr);

TEST_SUITE("Z80 flags") {

    TEST_CASE("Z80 sign flag can be enabled") {
        for (auto i = 0; i < 1000000; i++)
            target.setSignFlag(true);
        CHECK(target.isSignFlag() == true);
    }

    TEST_CASE("Z80 sign flag can be disabled") {
        target.setSignFlag(false);
        CHECK(target.isSignFlag() == false);
    }

}

#endif //Z80CPP_Z80TEST_CPP
