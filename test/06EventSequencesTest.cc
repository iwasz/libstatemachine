#include "GsmCommandAction.h"
#include "StateMachine.h"
#include "catch.hpp"
#include <cstring>
#include <etl/cstring.h>
#include <unistd.h>

enum MyStates { INITIAL, ALIVE, POWER_DOWN, X, Y, Z };

/**
 * @brief TEST_CASE
 */
TEST_CASE ("Events interleaved with run", "[Events]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine<> machine;
        auto &inputQueue = machine.getEventQueue ();

        bool cond = false;
        BoolCondition fakeCond (&cond);

        /* clang-format off */

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("INITIAL ENTRY"))
                ->transition (ALIVE)->when (anded (eq ("OK"), eq ("AT+CREG")));

        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"));

        /* clang-format on */

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Irrelevant
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 0);

        // Nie przejdzie, do alive, bo musi być "OK", AT+CREG, a jest tylko OK
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");

        // Simulate main loop calling machine.run () in short periods of time
        machine.run ();
        machine.run ();
        machine.run ();

        // Now modem puts some response in
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        // But main loop keeps going
        machine.run ();
        machine.run ();
        machine.run ();

        // And another input pops up - this time machine should do the transition.
        inputQueue.push_back ();
        inputQueue.back () = "AT+CREG";

        machine.run ();
        REQUIRE (inputQueue.size () == 0);

        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");

        // Teraz wykona entry na ALIVE i wyczyści input
        machine.run ();
        //        REQUIRE (inputQueue.size () == 0);
        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (gsmModemCommandsIssued.size () == 2);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");
        REQUIRE (gsmModemCommandsIssued[1] == "ALIVE ENTRY");
}
