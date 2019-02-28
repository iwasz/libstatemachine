#include "AndCondition.h"
#include "BoolCondition.h"
#include "GsmCommandAction.h"
#include "OrCondition.h"
#include "StateMachine.h"
#include "StringCondition.h"
#include "catch.hpp"
#include <cstring>
#include <etl/cstring.h>
#include <unistd.h>

enum MyStates { INITIAL, ALIVE, POWER_DOWN, X, Y, Z };

// using StateMachine = StateMachine<string>;

/**
 *
 */
TEST_CASE ("Ored, anded", "[AndCondition]")
{

        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        bool cond = false;
        BoolCondition fakeCond (&cond);

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("INITIAL ENTRY"))->transition (ALIVE)->when (anded (eq ("OK"), &fakeCond));
        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"))->exit (gsm ("ALIVE EXIT"))->transition (ALIVE)->when (ored (eq ("HEJ"), eq ("HOPS")));

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

        // Nie przejdzie, do alive, bo musi być "OK", oraz fake na true. Input skonsumowany.
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");
        //        REQUIRE (inputQueue.size () == 0);

        // Znów odkładamy "OK", ale i włączamy fake cond.
        inputQueue.push_back ();
        inputQueue.back () = "OK";
        cond = true;

        // Teraz przewjdzie do alive
        machine.run ();
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

        inputQueue.push_back ();
        inputQueue.back () = "KLOPS";

        machine.run ();
        //        REQUIRE (inputQueue.size () == 0);
        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (gsmModemCommandsIssued.size () == 2);

        inputQueue.push_back ();
        inputQueue.back () = "HEJ";

        // Zmienił stan a ALIVE na ALIVE
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        // Wykonał akcje i wyszedł
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 4);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");
        REQUIRE (gsmModemCommandsIssued[1] == "ALIVE ENTRY");
        REQUIRE (gsmModemCommandsIssued[2] == "ALIVE EXIT");
        REQUIRE (gsmModemCommandsIssued[3] == "ALIVE ENTRY");

        inputQueue.push_back ();
        inputQueue.back () = "HOPS";

        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 6);
        REQUIRE (gsmModemCommandsIssued[4] == "ALIVE EXIT");
        REQUIRE (gsmModemCommandsIssued[5] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        inputQueue.push_back ();
        inputQueue.back () = "HEJ";

        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 8);
        REQUIRE (gsmModemCommandsIssued[6] == "ALIVE EXIT");
        REQUIRE (gsmModemCommandsIssued[7] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (inputQueue.size () == 0);
}

/**
 * @brief TEST_CASE
 */
TEST_CASE ("Anded multi", "[slick]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        bool cond = false;
        BoolCondition fakeCond (&cond);

        machine.state (INITIAL, StateFlags::INITIAL)
                ->entry (gsm ("INITIAL ENTRY"))
                ->transition (ALIVE)
                ->when (anded (eq ("OK"), eq ("AT+CREG")));
        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"));

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
TEST_CASE ("Faulty ored", "[AndCondition]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        machine.state (INITIAL, StateFlags::INITIAL)
                ->entry (gsm ("INITIAL ENTRY"))
                ->transition (ALIVE)
                ->when (ored (anded (eq ("OK"), eq ("AT+CREG")), eq ("OR_THIS")));

        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"));

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
TEST_CASE ("Faulty anded", "[AndCondition]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        machine.state (INITIAL, StateFlags::INITIAL)
                ->entry (gsm ("INITIAL ENTRY"))
                ->transition (ALIVE)
                ->when (anded (eq ("THIS"), anded (eq ("OK"), eq ("AT+CREG"))));

        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"));

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
TEST_CASE ("And order doesn't care", "[AndCondition]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        machine.state (INITIAL, StateFlags::INITIAL)
                ->entry (gsm ("INITIAL ENTRY"))
                ->transition (ALIVE)
                ->when (anded (eq ("THIS"), anded (eq ("OK"), eq ("AT+CREG"))));

        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"));

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
        REQUIRE (machine.currentState->getLabel () == ALIVE);
}
