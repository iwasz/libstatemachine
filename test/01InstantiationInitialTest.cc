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

// using namespace sm;

enum MyStates { INITIAL, ALIVE, POWER_DOWN, X, Y, Z };

/**
 * @brief TEST_CASE
 */
TEST_CASE ("Pierwszy slick", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine<> machine;
        auto &inputQueue = machine.getEventQueue ();

        /* clang-format off */
        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("AT"))
                ->transition (ALIVE)->when (eq ("OK"))->then (gsm ("XYZ"));

        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"))
                ->transition (POWER_DOWN)->when (eq ("OK"))->then (gsm ("XYZ"));

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
        inputQueue.back () = "OK";

        inputQueue.push_back ();
        inputQueue.back () = "7867868";

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
        inputQueue.back () = "OK";
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);

        REQUIRE (gsmModemCommandsIssued.size () == 5);
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH"); // Z exit action stanu aliveState
        REQUIRE (gsmModemCommandsIssued[4] == "XYZ");  // Z transition action między alive a powerdown

        REQUIRE (inputQueue.size () == 0);
}

/**
 * @brief Testuje co się stanie jak na wejściu pojawi się dużo niepotrzebnego śmiecia.
 */
TEST_CASE ("Irrelevant input slick", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("AT"))->transition (ALIVE)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"))->transition (POWER_DOWN)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (POWER_DOWN);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        /*---------------------------------------------------------------------------*/
        /* Drugie uruchomienie maszyny. Modem wysyła niepotrzebne dane.              */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        inputQueue.back () = "VERSION 123";

        machine.run ();
        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        //        REQUIRE (inputQueue.size () == 0);

        /*---------------------------------------------------------------------------*/
        /* Drugie uruchomienie maszyny. Modem wysyła kilka linii do zignorowania.    */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        inputQueue.back () = "string1";

        inputQueue.push_back ();
        inputQueue.back () = "string2";

        inputQueue.push_back ();
        inputQueue.back () = "string3";

        //        REQUIRE (inputQueue.size () == 3);
        machine.run ();
        //        REQUIRE (inputQueue.size () == 2);
        machine.run ();
        //        REQUIRE (inputQueue.size () == 1);
        machine.run ();
        //        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        //        REQUIRE (inputQueue.size () == 0);

        /*---------------------------------------------------------------------------*/
        /* Modem wysyła kilka linii do zignorowania, a na końcu OK                   */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        inputQueue.back () = "string1";

        inputQueue.push_back ();
        inputQueue.back () = "string2";

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        // Tu się zmienia stan
        machine.run ();
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        // Tu się uruchamiają akcje
        machine.run ();
        //        REQUIRE (inputQueue.size () == 0);
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");  // To jest pierwsza komenda, ktorą maszyna wysłała z entry action ze stanu initialState.
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ"); // To jest transition action.
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF"); // A to dodała entry action ze stanu aliveState

        // A tu się nic nie dzieje
        machine.run ();
        REQUIRE (inputQueue.size () == 0);
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ");
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF");
}

/**
 * @brief Dużo wejścia, kilka lini na raz, każda zmienia stan maszyny.
 */
TEST_CASE ("Multiple relevant input slick", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("AT"))->transition (ALIVE)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"))->transition (POWER_DOWN)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (POWER_DOWN);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        // Irrelevant
        inputQueue.push_back ();
        inputQueue.back () = "VERSION !@#";

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        /*
         * Wykona entryAction z initialState.entryAction (stąd jest "AT")
         * Pobierze z koleki wejściowej "OK"
         * Sprawdzi warunek i zmieni stan na aliveState
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        /*
         * Wykona akcję transitionAction ("XYZ")
         * Wykona akcję entryAction ze stanu aliveState
         * Pobierze input "VERSION !@#"
         * Sprawdzi, że żaden warunek nie jest spełniony, ale wejście zostało skonsumowane, więc wyczyści kolejkę
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ");
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF");

        // Kolejka jest czyszcona zgodnie z nowym sposobem po każdym przejsicu stanowym, które go warunki czytały (konsumowały) wejście.
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        /*
         * Nie wykona żadnej akcji
         * Pobierze "OK" z kolejki
         * Znajdzie przejscie i zmieni stan na powerDownState
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);

        /*
         * Wykona exitAction z aliveState
         * Wykona transition action.
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 5);
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH"); // Z exit action stanu aliveState
        REQUIRE (gsmModemCommandsIssued[4] == "XYZ");  // Z transition action między alive a powerdown
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);
}

/**
 * @brief Testuje opóźnienia.
 */
TEST_CASE ("Delays slick", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        /* clang-format off */
        machine.state (INITIAL, StateFlags::INITIAL)->entry (and_action (delayMs (3), gsm ("AT")))
                ->transition (ALIVE)->when (eq ("OK"))->then (and_action (delayMs (3), gsm ("XYZ")));

        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"))
                ->transition (POWER_DOWN)->when (eq ("OK"))->then (and_action (delayMs (3), gsm ("XYZ")));

        machine.state (POWER_DOWN);
        /* clang-format on */

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        // Irrelevant
        inputQueue.push_back ();
        inputQueue.back () = "VERSION !@#";

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 0);

        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 0);

        usleep (4 * 1000);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);

        inputQueue.push_back ();
        inputQueue.back () = "OK";
        usleep (4 * 1000);
        machine.run ();

        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ");
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF");
        // Załapał OK z poprzedniego.

        inputQueue.push_back ();
        inputQueue.back () = "OK";
        usleep (4 * 1000);
        machine.run ();

        //        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);
        // Wykona akcje.
        //        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 4); // 4 bo było wyjście z alive, i exit Action się wykonało
        REQUIRE (gsmModemCommandsIssued[0] == "AT");
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ");
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF");
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH"); // Z exit action stanu aliveState
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 4);
        usleep (4 * 1000);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 5);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ");
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF");
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH"); // Z exit action stanu aliveState
        REQUIRE (gsmModemCommandsIssued[4] == "XYZ");  // Z transition action między alive a powerdown
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);
}

// class FakeCondition : public Condition {
// public:
//        FakeCondition (bool *b) : b (b) {}

// private:
//        bool check (const char *data) const
//        {
//                return *b;
//        }

//        bool *b;
//};

/**
 * @brief Testuje warunek, który nie sprawdza wejść, tylko coś zewnętrznego (zmienną).
 */
TEST_CASE ("Non input condition slick", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        bool cond = false;
        BoolCondition fakeCond (&cond);

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("INITIAL ENTRY"))->transition (ALIVE)->when (&fakeCond);
        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"))->transition (POWER_DOWN)->when (eq ("OK"));
        machine.state (POWER_DOWN);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Irrelevant
        inputQueue.push_back ();
        inputQueue.back () = "VERSION !@#";

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 0);

        /*
         * Wykona entryAction z initialState.entryAction
         * Pobierze z koleki wejściowej "VERSION !@#"
         * Sprawdzi warunek fake i wyjdzie
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "INITIAL ENTRY");
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        // Nic się nie stanie.
        // W kolejce wejściowej cały czas na froncie jest "VERSION !@#"
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "INITIAL ENTRY");
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        /*
         * Sprawdzi warunek fake i zmieni stan na aliveState
         * W kolejce wejściowej cały czas "VERSION !@#", nic się nie dzieje z kolejką, bo fake nie konsumuje.
         */
        cond = true;
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "INITIAL ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        /*
         * Wykona akcję entry action z aliveState
         * Pobierze z koleki wejściowej "VERSION !@#"
         * Żaden warunek nie przejdzie, ale skonsumuje input, wyczyści kolejkę
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 2);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");
        REQUIRE (gsmModemCommandsIssued[1] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        // Kolejka jest czyszcona zgodnie z nowym sposobem.
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        /*
         * Nie wykona akcji.
         * Pobierze z kolejki "OK"
         * Warunek się spełni, przejdzie do powerDownState
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 2);
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);
}

/**
 * @brief Testuje czy działa przejśącie stanowe do samego siebie i czy za każdym razem wykonują się wszystkie akcje.
 */
TEST_CASE ("Transition to myself slick", "[Instantiation]")
{

        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        static bool bbb = true;
        BoolCondition fakeCond (&bbb);

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("INITIAL ENTRY"))->transition (ALIVE)->when (&fakeCond);
        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"))->exit (gsm ("ALIVE EXIT"))->transition (ALIVE)->when (&fakeCond);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 0);

        /*
         * Wykona entryAction z initialState.entryAction
         * Pobierze z koleki wejściowej "VERSION !@#"
         * Sprawdzi warunek fake i wyjdzie
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "INITIAL ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 2);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");
        REQUIRE (gsmModemCommandsIssued[1] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 4);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");
        REQUIRE (gsmModemCommandsIssued[1] == "ALIVE ENTRY");
        REQUIRE (gsmModemCommandsIssued[2] == "ALIVE EXIT");
        REQUIRE (gsmModemCommandsIssued[3] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 6);
        REQUIRE (gsmModemCommandsIssued[4] == "ALIVE EXIT");
        REQUIRE (gsmModemCommandsIssued[5] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 8);
        REQUIRE (gsmModemCommandsIssued[6] == "ALIVE EXIT");
        REQUIRE (gsmModemCommandsIssued[7] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        REQUIRE (inputQueue.size () == 0);
}

/**
 *
 */
TEST_CASE ("Negated", "[Instantiation]")
{

        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("INITIAL ENTRY"))->transition (ALIVE)->when (ne ("OK"));
        machine.state (ALIVE)
                ->entry (gsm ("ALIVE ENTRY"))
                ->exit (gsm ("ALIVE EXIT"))
                ->transition (ALIVE)
                ->when (anded (ne ("HEJ"), ne ("HOPS")));

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

        // Nie przejdzie, do alive, bo musi być con innego niż OK
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");

        inputQueue.push_back ();
        inputQueue.back () = "NOOK";

        // Teraz przewjdzie do alive
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (gsmModemCommandsIssued.size () == 2);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");
        REQUIRE (gsmModemCommandsIssued[1] == "ALIVE ENTRY");

        REQUIRE (inputQueue.size () == 0);
        inputQueue.push_back ();
        inputQueue.back () = "HEJ";

        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (gsmModemCommandsIssued.size () == 2);

        inputQueue.push_back ();
        inputQueue.back () = "HOPS";

        //        machine.run ();
        //        machine.run ();
        //        REQUIRE (machine.currentState->getLabel () == ALIVE);
        //        REQUIRE (gsmModemCommandsIssued.size () == 2);
        //        REQUIRE (inputQueue.size () == 0);
}

/**
 *
 */
TEST_CASE ("Time passes", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        TimeCounter tc;
        StateMachine machine;
        machine.setTimeCounter (&tc);

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("INITIAL ENTRY"))->transition (ALIVE)->when (msPassed (10, &tc));
        machine.state (ALIVE)->entry (gsm ("ALIVE ENTRY"));

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 0);

        // Nie przejdzie, do alive, bo musi być con innego niż OK
        machine.run ();
        machine.run ();
        machine.run ();
        machine.run ();
        machine.run ();
        machine.run ();
        machine.run ();
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        usleep (15 * 1000);
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
}

struct FakeAction : public Action<string> {
        virtual ~FakeAction () override = default;

        virtual bool run (string const &event) override
        {
                if (!event.empty ()) {
                        lastInput = event.c_str ();
                }

                return true;
        }

        std::string lastInput;
};

/**
 * @brief Testuje czy faktycznie "relevent" input trafia do akcji, a niepotrzebne wejącia są odrzucane
 */
TEST_CASE ("Irrelevant and action", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        FakeAction fakeAction;

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        machine.state (INITIAL, StateFlags::INITIAL)
                ->entry (gsm ("AT"))
                ->transition (ALIVE)
                ->when (eq ("OK", StripInput::STRIP, InputRetention::RETAIN_INPUT));
        machine.state (ALIVE)->entry (&fakeAction)->transition (POWER_DOWN)->when (eq ("OK"));
        machine.state (POWER_DOWN);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        inputQueue.push_back ();
        inputQueue.back () = "string1";

        inputQueue.push_back ();
        inputQueue.back () = "string2";

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        machine.run ();
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (fakeAction.lastInput == "OK");
}

template <typename Q> struct FakeAction22 : public Action<string> {

        FakeAction22 (Q &q) : inputQueue (q) {}
        virtual ~FakeAction22 () = default;

        virtual bool run (string const &)
        {
                inputQueue.push_back ();
                inputQueue.back () = "string1";
                return true;
        }

        Q &inputQueue;
};

/**
 * @brief Testuje czy jeśli akcja spowoduje pojawienie się nowego inputu (po to są akcje),
 * i czy nie zostanie on wyczyszczony przedwcześnie.
 */
TEST_CASE ("Action that causes an input", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        FakeAction22 fakeAction (inputQueue);

        machine.state (INITIAL, StateFlags::INITIAL)->transition (ALIVE)->when (eq ("OK"));
        machine.state (ALIVE)->entry (and_action (&fakeAction, delayMs (1)))->transition (POWER_DOWN)->when (eq ("string1"));
        machine.state (POWER_DOWN);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        // Przejście stanowe
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);

        // Wykonanie akcji (między innymi entry do ALIVE).
        machine.run ();
        usleep (2 * 1000);
        machine.run ();

        // Zmienił stan na power DOWN, czyli wygenerowany input trafił do kolejki i został użyty do warunku przejścia i zaakceptowany.
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN);
        //        REQUIRE (inputQueue.size () == 1);
        //        REQUIRE (inputQueue.front ()->data == std::string ("string1"));
}

/**
 * @brief TEST_CASE
 */
TEST_CASE ("Global transition", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        machine.transition (INITIAL)->when (eq ("RESET"))->then (gsm ("RRR"));
        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("AT"))->transition (ALIVE)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"))->transition (POWER_DOWN)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (POWER_DOWN);

        /*---------------------------------------------------------------------------*/

        REQUIRE (gsmModemCommandsIssued.empty ());
        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");

        /*---------------------------------------------------------------------------*/

        // Symulujemy, że modem odpowiedział "OK", co pojawiło się na kolejce danych we. Maszyna
        // monitoruje tą kolejkę.
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");  // To jest pierwsza komenda, ktorą maszyna wysłała z entry action ze stanu initialState.
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ"); // To jest transition action.
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF"); // A to dodała entry action ze stanu aliveState

        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        inputQueue.back () = "RESET";

        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 6);
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH");
        REQUIRE (gsmModemCommandsIssued[4] == "RRR");
        REQUIRE (gsmModemCommandsIssued[5] == "AT");
}

/**
 *
 */
#if 0
TEST_CASE ("Multi Ored", "[Instantiation]")
{

        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        bool cond = false;
        BoolCondition fakeCond (&cond);

        machine.state (INITIAL, true)->transition (ALIVE)->when (ored (eq ("A"), eq ("B"), eq ("C"), eq ("D"), eq ("E")));
        machine.state (ALIVE);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Irrelevant
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);

        inputQueue.push_back ();
        inputQueue.back () = "A";

        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
}
#endif

/**
 * @brief TEST_CASE
 */
TEST_CASE ("Machine reset", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("AT"))->transition (ALIVE)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"))->transition (POWER_DOWN)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (POWER_DOWN);
        machine.state (X)->entry (gsm ("XXXX"))->transition (Y)->when (eq ("YYY"));
        machine.state (Y)->entry (gsm ("PIPIP"));

        /*---------------------------------------------------------------------------*/

        REQUIRE (gsmModemCommandsIssued.empty ());
        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");

        /*---------------------------------------------------------------------------*/

        // Symulujemy, że modem odpowiedział "OK", co pojawiło się na kolejce danych we. Maszyna
        // monitoruje tą kolejkę.
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");  // To jest pierwsza komenda, ktorą maszyna wysłała z entry action ze stanu initialState.
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ"); // To jest transition action.
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF"); // A to dodała entry action ze stanu aliveState

        /*---------------------------------------------------------------------------*/

        // NIE uruchomi żadnych akcji.
        machine.setInitialState (X);
        machine.reset ();

        machine.run ();
        machine.run ();

        REQUIRE (machine.currentState->getLabel () == X);
        REQUIRE (inputQueue.size () == 0);

        inputQueue.push_back ();
        inputQueue.back () = "YYY";

        machine.run ();
        machine.run ();

        REQUIRE (machine.currentState->getLabel () == Y);
        REQUIRE (gsmModemCommandsIssued.size () == 5);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ");
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF");
        REQUIRE (gsmModemCommandsIssued[3] == "XXXX");
        REQUIRE (gsmModemCommandsIssued[4] == "PIPIP");
}

TEST_CASE ("Global transition only", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        machine.transition (INITIAL)->when (eq ("RESET"))->then (gsm ("RRR"));
        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("AT"))->transition (ALIVE)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"));
        machine.state (POWER_DOWN);

        /*---------------------------------------------------------------------------*/

        REQUIRE (gsmModemCommandsIssued.empty ());
        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");

        /*---------------------------------------------------------------------------*/

        // Symulujemy, że modem odpowiedział "OK", co pojawiło się na kolejce danych we. Maszyna
        // monitoruje tą kolejkę.
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");  // To jest pierwsza komenda, ktorą maszyna wysłała z entry action ze stanu initialState.
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ"); // To jest transition action.
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF"); // A to dodała entry action ze stanu aliveState

        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        inputQueue.back () = "RESET";

        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 6);
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH");
        REQUIRE (gsmModemCommandsIssued[4] == "RRR");
        REQUIRE (gsmModemCommandsIssued[5] == "AT");
}

/**
 * @brief TEST_CASE
 */
TEST_CASE ("Global transition first", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        /* clang-format off */
        machine.transition (INITIAL, TransitionPriority::RUN_FIRST)->when (eq ("RESET"))->then (gsm ("RRR"));

        machine.state (INITIAL, StateFlags::INITIAL)->entry (gsm ("AT"))
                ->transition (ALIVE)->when (eq ("OK"))->then (gsm ("XYZ"));

        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"))
                ->transition (POWER_DOWN)->when (eq ("OK"))->then (gsm ("XYZ"));

        machine.state (POWER_DOWN);
        /* clang-format on */

        /*---------------------------------------------------------------------------*/

        REQUIRE (gsmModemCommandsIssued.empty ());
        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");

        /*---------------------------------------------------------------------------*/

        // Symulujemy, że modem odpowiedział "OK", co pojawiło się na kolejce danych we. Maszyna
        // monitoruje tą kolejkę.
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE);
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");  // To jest pierwsza komenda, ktorą maszyna wysłała z entry action ze stanu initialState.
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ"); // To jest transition action.
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF"); // A to dodała entry action ze stanu aliveState

        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        inputQueue.back () = "RESET";

        inputQueue.push_back ();
        inputQueue.back () = "OK";

        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL);
        REQUIRE (gsmModemCommandsIssued.size () == 6);
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH");
        REQUIRE (gsmModemCommandsIssued[4] == "RRR");
        REQUIRE (gsmModemCommandsIssued[5] == "AT");
}

/*****************************************************************************/

#if 0
TEST_CASE ("Lambdas", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StateMachine machine;
        auto &inputQueue = machine.getEventQueue ();

        int var = 0;

        /* clang-format off */
        machine.state (INITIAL, true)
                ->entry ([&var] (const char *, void *) { var = 2; return true; })
                ->transition (ALIVE)->when (eq ("OK"))->then ([&var] (const char *, void *) { var = 555; return true; });

        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"))->transition (POWER_DOWN)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (POWER_DOWN);
        /* clang-format on */

        /*---------------------------------------------------------------------------*/

        REQUIRE (var == 0);
        machine.run ();
        machine.run ();
        REQUIRE (var == 2);

        // Symulujemy, że modem odpowiedział "OK", co pojawiło się na kolejce danych we. Maszyna
        // monitoruje tą kolejkę.
        inputQueue.push_back ();
        inputQueue.back () = "OK";

        machine.run ();
        machine.run ();
        REQUIRE (var == 555);
        REQUIRE (machine.currentState->getLabel () == ALIVE);
}

/*****************************************************************************/

 TEST_CASE ("Arguments", "[Instantiation]")
{
        gsmModemCommandsIssued.clear ();

        StringQueue inputQueue (STRING_QUEUE_SIZE);
        StateMachine machine (&inputQueue);

        int var = 0;

        /* clang-format off */
        machine.state (INITIAL, true)
                ->entry ([&var] (const char *, void *) { var = 2; return true; })
                ->transition (ALIVE)->when (eq ("OK"))->then ([&var] (const char *, void *arg) { var = *static_cast <int *> (arg); return true;
                });

        machine.state (ALIVE)->entry (gsm ("POWEROFF"))->exit (gsm ("BLAH"))->transition (POWER_DOWN)->when (eq ("OK"))->then (gsm ("XYZ"));
        machine.state (POWER_DOWN);
        /* clang-format on */

        /*---------------------------------------------------------------------------*/

        REQUIRE (var == 0);
        machine.run ();
        machine.run ();
        REQUIRE (var == 2);

        // Symulujemy, że modem odpowiedział "OK", co pojawiło się na kolejce danych we. Maszyna
        // monitoruje tą kolejkę.
        inputQueue.push_back ();
        inputQueue.back () = "OK";
        int i = 777;
        el->arg = &i;

        machine.run ();
        machine.run ();
        REQUIRE (var == 777);
        REQUIRE (machine.currentState->getLabel () == ALIVE);
}

#endif
