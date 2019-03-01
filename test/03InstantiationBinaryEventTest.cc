#include "AndCondition.h"
#include "BoolCondition.h"
#include "DelayAction.h"
#include "GsmCommandAction.h"
#include "OrCondition.h"
#include "StateMachine.h"
#include "StringCondition.h"
#include "TimePassedCondition.h"
#include "catch.hpp"
#include <cstdint>
#include <cstring>
#include <etl/cstring.h>
#include <etl/vector.h>
#include <unistd.h>

// using namespace sm;

enum MyStates { INITIAL, ALIVE, POWER_DOWN, X, Y, Z };

using EventType = etl::vector<uint8_t, 64>;

/**
 * @brief TEST_CASE
 */
TEST_CASE ("First", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine<EventType> machine;
        auto &inputQueue = machine.getEventQueue ();

        /* clang-format off */
        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm <EventType>("AT"))
                ->transition (ALIVE)->when (eq <EventType>("OK"))->then (gsm<EventType> ("XYZ"));

        machine.state (ALIVE)->entry (gsm <EventType>("POWEROFF"))->exit (gsm <EventType>("BLAH"))
                ->transition (POWER_DOWN)->when (eq<EventType> ("OK"))->then (gsm <EventType>("XYZ"));

        machine.state (POWER_DOWN);
        /* clang-format on */

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
        inputQueue.back ().resize (2);
        inputQueue.back ()[0] = 'O';
        inputQueue.back ()[1] = 'K';

        inputQueue.push_back ();
        inputQueue.back ().resize (6);
        inputQueue.back ()[0] = '7';
        inputQueue.back ()[1] = '8';
        inputQueue.back ()[2] = '6';
        inputQueue.back ()[3] = '7';
        inputQueue.back ()[4] = '8';
        inputQueue.back ()[5] = '6';

        // Trzecie uruchomienie maszyny, transition złapała odpowiedź "OK" i zadecydowała o zmianie stanu.
        machine.run ();

        // Akcje.
        machine.run ();
        REQUIRE (machine.currentState);
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
        inputQueue.back ().resize (2);
        inputQueue.back ()[0] = 'O';
        inputQueue.back ()[1] = 'K';
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);

        REQUIRE (gsmModemCommandsIssued.size () == 5);
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH"); // Z exit action stanu aliveState
        REQUIRE (gsmModemCommandsIssued[4] == "XYZ");  // Z transition action między alive a powerdown

        REQUIRE (inputQueue.size () == 0);
}
