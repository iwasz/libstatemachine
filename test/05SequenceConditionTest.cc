#include "BoolCondition.h"
#include "GsmCommandAction.h"
#include "OrCondition.h"
#include "SequenceCondition.h"
#include "StateMachine.h"
#include "StringCondition.h"
#include "catch.hpp"
#include <cstring>
#include <etl/cstring.h>
#include <unistd.h>

enum MyStates { INITIAL, ALIVE, POWER_DOWN, X, Y, Z };

/**
 * @brief TEST_CASE
 */
TEST_CASE ("Seq multi", "[SeqCondition]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine<> machine;
        auto &inputQueue = machine.getEventQueue ();

        bool cond = false;
        BoolCondition fakeCond (&cond);

        /* clang-format off */

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("INITIAL ENTRY"))
                ->transition (ALIVE)->when (seq (*eq ("OK"), *eq ("AT+CREG")));

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

        //        REQUIRE (inputQueue.size () == 0);
        // Znów odkładamy "OK", ale i włączamy fake cond.
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        inputQueue.push_back ();
        inputQueue.back () = "BABABA";

        // Nie przejdzie. Wprawdzie są 2 inputy,ale drugi jest zły.
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");

        //        REQUIRE (inputQueue.size () == 0);
        // Znów odkładamy "OK", ale i włączamy fake cond.
        inputQueue.push_back ();
        inputQueue.back () = "OK";

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

/**
 * This one checks for a particular bug that I found in OrCondition when AndCondition
 * was nested inside.
 */
TEST_CASE ("Faulty ored with seq", "[SeqCondition]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        /* clang-format off */

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("INITIAL ENTRY"))
                ->transition (ALIVE)->when (ored (seq (*eq ("OK"), *eq ("AT+CREG")), eq ("OR_THIS")));

        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"));

        /* clang-format on */

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        REQUIRE (gsmModemCommandsIssued.size () == 0);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        inputQueue.push_back ();
        inputQueue.back () = "AT+CREG";

        /*
         * Here it should transition to ALIVE state, but it failed due to faulty implementation.
         * Basically OrConditiopn called AndCondition::checkImpl instead of full AndCondition::check.
         */
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
}

/**
 * The same (or very similar) bug as one above affected the AndCondition implementation.
 * Nested "anded" and "ored" didn't work at all.
 */
TEST_CASE ("Faulty seq", "[SeqCondition]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        /* clang-format off */

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("INITIAL ENTRY"))
                ->transition (ALIVE)->when (seq (*eq ("THIS"), *seq (*eq ("OK"), *eq ("AT+CREG"))));

        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"));

        /* clang-format on */

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        REQUIRE (gsmModemCommandsIssued.size () == 0);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");

        inputQueue.push_back ();
        inputQueue.back () = "THIS";

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        inputQueue.push_back ();
        inputQueue.back () = "AT+CREG";

        /*
         * Here it should transition to ALIVE state, but it failed due to faulty implementation.
         * Basically OrConditiopn called AndCondition::checkImpl instead of full AndCondition::check.
         */

        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
}

/**
 * Checks if AndCondition cares about events order. It should NOT. In previous example
 * I put 3 events in order the condition expected (THIS, OK and AT+CREG). This time I check
 * changed order of events.
 */
TEST_CASE ("Seq order is significant", "[SeqCondition]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        /* clang-format off */

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("INITIAL ENTRY"))
                ->transition (ALIVE)->when (seq (*eq ("THIS"), *seq (*eq ("OK"), *eq ("AT+CREG"))));

        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"));

        /* clang-format on */

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        REQUIRE (gsmModemCommandsIssued.size () == 0);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        inputQueue.push_back ();
        inputQueue.back () = "AT+CREG";

        inputQueue.push_back ();
        inputQueue.back () = "THIS";

        /*
         * Here it should transition to ALIVE state, but it failed due to faulty implementation.
         * Basically OrConditiopn called AndCondition::checkImpl instead of full AndCondition::check.
         */

        machine.run ();
        REQUIRE (machine.currentState->getLabel () != ALIVE);
}
