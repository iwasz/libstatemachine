#include "GsmCommandAction.h"
#include "StateMachine.h"
#include "StringCondition.h"
#include "catch.hpp"
#include <cstring>
#include <etl/cstring.h>

using string = etl::string<64>;

/**
 * @brief TEST_CASE
 */
TEST_CASE ("First instantiation", "[Instantiation]")
{
        REQUIRE (true);
        //        StateMachine<string> machine;
}

enum MyStates { INITIAL, ALIVE, POWER_DOWN, X, Y, Z };
static constexpr size_t STRING_QUEUE_SIZE = 16;

/**
 * @brief TEST_CASE
 */
TEST_CASE ("Pierwszy slick", "[slick]")
{
        gsmModemCommandsIssued.clear ();

        StringQueue inputQueue (STRING_QUEUE_SIZE);
        StateMachine machine (&inputQueue);

        machine.state (INITIAL, true)->entry (gsm ("AT"))->transition (ALIVE)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"))->transition (POWER_DOWN)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (POWER_DOWN);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Maszyna stanów nie wysłała żadnej komendy do modemu
        REQUIRE (gsmModemCommandsIssued.empty ());

        // To będzie się cały czas odpalało w pętli. Tu symulujemy, że odpaliło się pierwszy raz.
        // Czyli maszyna przejdzie ze stanu nieustalonego do stanu początkowego "initialState".
        machine.run ();

        // Drugie wywołanie żeby odpalić akcje.
        machine.run ();

        // Maszyna wysłała pierwszą komendę, bo wykonała się entry action z initialState.
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");

        /*---------------------------------------------------------------------------*/
        /* Drugie uruchomienie maszyny.                                              */
        /*---------------------------------------------------------------------------*/

        // Następne uruchomienie maszyny bez danych wejściowych nie powinno zmienić stanu i wywołać żadnej akcji
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        // Symulujemy, że modem odpowiedział "OK", co pojawiło się na kolejce danych we. Maszyna
        // monitoruje tą kolejkę.
        inputQueue.push_back ();
        char *el = inputQueue.back ();
        strcpy (el, "OK");

        // Trzecie uruchomienie maszyny, transition złapała odpowiedź "OK" i zadecydowała o zmianie stanu.
        machine.run ();

        // Akcje.
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        // Jeżeli maszyna stanów dokonała zmiany stanu, to automatycznie powinna zdjąć element z kolejki.
        //        REQUIRE (inputQueue.size () == 0);

        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");  // To jest pierwsza komenda, ktorą maszyna wysłała z entry action ze stanu initialState.
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ"); // To jest transition action.
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF"); // A to dodała entry action ze stanu aliveState

        /*---------------------------------------------------------------------------*/

        // Symulujemy znów odpowieddź od modemu, że OK
        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el, "OK");
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);

        REQUIRE (gsmModemCommandsIssued.size () == 5);
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH"); // Z exit action stanu aliveState
        REQUIRE (gsmModemCommandsIssued[4] == "XYZ");  // Z transition action między alive a powerdown

        REQUIRE (inputQueue.size () == 0);
}
