# Wymagaj tej wersji - dopier w niej jest nowe makro FindPkgConfig
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.0 FATAL_ERROR)

PROJECT (libstatemachine)
SET (CMAKE_VERBOSE_MAKEFILE OFF)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

SET(CMAKE_C_FLAGS "-std=gnu99 -Wall" CACHE INTERNAL "c compiler flags")
SET(CMAKE_CXX_FLAGS "-std=c++17 -Wall" CACHE INTERNAL "cxx compiler flags")

ADD_DEFINITIONS ("-DUNIT_TEST -DLIB_STATE_MACHINE_DEFAULT_EVENT_TYPE=string")

SET (CATCH2_INCLUDE "deps/Catch2/single_include/catch2")
INCLUDE_DIRECTORIES("${CATCH2_INCLUDE}")

SET (ETL_INCLUDE "../etl/include")
INCLUDE_DIRECTORIES("${ETL_INCLUDE}")

INCLUDE_DIRECTORIES("src")
INCLUDE_DIRECTORIES("test")
INCLUDE_DIRECTORIES("../libmicro/src/")
    
ADD_EXECUTABLE (unittest
    "${CATCH2_INCLUDE}/catch.hpp"

    "../libmicro/src/collection/Queue.h"
    "../libmicro/src/ErrorHandler.h"
    "../libmicro/src/ErrorHandler.c"
    "../libmicro/src/Timer.h"
    "../libmicro/src/TimerUnit.cc"

    "src/StringQueue.h"

    "src/Action.h"
    "src/Condition.h"
    "src/AndCondition.h"
    "src/SequenceCondition.h"
    "src/OrCondition.h"
    "src/BoolCondition.h"
    "src/StringCondition.h"
    "src/State.h"
    "src/StateMachine.h"
    "src/StateMachineTypes.h"
    "src/Transition.h"
    "src/TimeCounter.h"
    "src/TimePassedCondition.h"
    "src/DelayAction.h"
    "src/BeginsWithCondition.h"
    "src/IntegerAction.h"
    "src/IntegerCondition.h"
    "src/LikeCondition.h"
    "src/NotCondition.h"
    "src/StateChangeAction.h"
    "src/StateCondition.h"

    "test/etl_profile.h"
    "test/GsmCommandAction.h"
    "test/GsmCommandAction.cc"
    "test/00CatchInit.cc"
    "test/01InstantiationInitialTest.cc"
    "test/02AndConditionTest.cc"
    "test/03InstantiationBinaryEventTest.cc"
    "test/04LikeConditionTest.cc"
    "test/05SequenceConditionTest.cc"
    "test/06EventSequencesTest.cc"
    "test/07FunctionParameters.cc"
    "test/08InstantiationEventArgumentsTest.cc"
    "test/09DeferTest.cc"
)

