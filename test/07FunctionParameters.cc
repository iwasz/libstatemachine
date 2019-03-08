#include "GsmCommandAction.h"
#include "StateMachine.h"
#include "catch.hpp"
#include <cstring>
#include <etl/cstring.h>
#include <functional>
#include <iostream>
#include <unistd.h>

enum MyStates { INITIAL, ALIVE, POWER_DOWN, X, Y, Z };

void func (string const &s) { std::cerr << s.c_str () << std::endl; }

/**
 * @brief TEST_CASE
 */
TEST_CASE ("?", "[FunctionParameters]")
{
        // TODO
}
