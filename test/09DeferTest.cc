#include "AndCondition.h"
#include "BoolCondition.h"
#include "DelayAction.h"
#include "GsmCommandAction.h"
#include "OrCondition.h"
#include "StateMachine.h"
#include "StringCondition.h"
#include "TimePassedCondition.h"
#include "catch.hpp"
#include <cstring>
#include <etl/cstring.h>
#include <unistd.h>

enum MyStates { INITIAL, WAIT_FOR_COMMAND, DO_WORK, POWER_DOWN };

/**
 * @brief TEST_CASE
 */
TEST_CASE ("Simple deferral", "[Defer]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine<> machine;
        auto &inputQueue = machine.getEventQueue ();

        /* clang-format off */
        machine.state (INITIAL, StateFlags::INITIAL)
                ->transition (WAIT_FOR_COMMAND)->when (eq ("OK"));

        machine.state (WAIT_FOR_COMMAND)
                ->transition (DO_WORK)->when (eq ("DO"))
                ->transition (POWER_DOWN)->when (eq ("OFF"))->defer (DO_WORK);

        machine.state (DO_WORK)->entry (gsm ("WORK"))
                ->transition (WAIT_FOR_COMMAND)->when (eq ("OK"));

        machine.state (POWER_DOWN);
        /* clang-format on */

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        REQUIRE (gsmModemCommandsIssued.empty ());
        machine.run ();

        REQUIRE (machine.currentState->getLabel () == INITIAL);

        inputQueue.push_back ();
        inputQueue.back () = "OK";
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState);
        REQUIRE (machine.currentState->getLabel () == WAIT_FOR_COMMAND);

        inputQueue.push_back ();
        inputQueue.back () = "DO";
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState);
        REQUIRE (machine.currentState->getLabel () == DO_WORK);

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        inputQueue.push_back ();
        inputQueue.back () = "OFF";
        machine.run ();
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState);
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);

        /*---------------------------------------------------------------------------*/
        /* One more time, but now events in different order                          */
        /*---------------------------------------------------------------------------*/

        machine.reset ();
        inputQueue.clear ();
        gsmModemCommandsIssued.clear ();

        machine.run ();

        REQUIRE (machine.currentState->getLabel () == INITIAL);

        inputQueue.push_back ();
        inputQueue.back () = "OK";
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState);
        REQUIRE (machine.currentState->getLabel () == WAIT_FOR_COMMAND);

        inputQueue.push_back ();
        inputQueue.back () = "DO";
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState);
        REQUIRE (machine.currentState->getLabel () == DO_WORK);

        inputQueue.push_back ();
        inputQueue.back () = "OFF";

        inputQueue.push_back ();
        inputQueue.back () = "OK";
        machine.run ();
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState);
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);

}


/**
 * @brief TEST_CASE
 */
TEST_CASE ("Global deferral", "[Defer]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine<> machine;
        auto &inputQueue = machine.getEventQueue ();

        /* clang-format off */
        machine.state (INITIAL, StateFlags::INITIAL)
                ->transition (WAIT_FOR_COMMAND)->when (eq ("OK"));

        machine.state (WAIT_FOR_COMMAND)
                ->transition (DO_WORK)->when (eq ("DO"))
                ->transition (POWER_DOWN)->when (eq ("OFF"))->defer (DO_WORK, true);

        machine.state (DO_WORK)->entry (gsm ("WORK"))
                ->transition (WAIT_FOR_COMMAND)->when (eq ("OK"));

        machine.state (POWER_DOWN);
        /* clang-format on */

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        inputQueue.back () = "OFF";

        REQUIRE (gsmModemCommandsIssued.empty ());
        machine.run ();

        REQUIRE (machine.currentState->getLabel () == INITIAL);

        inputQueue.push_back ();
        inputQueue.back () = "OK";
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState);
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);
}
