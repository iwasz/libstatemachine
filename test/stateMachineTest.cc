/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../src/StateMachine.h"
#include "../src/DelayAction.h"
#include "GsmCommandAction.h"

uint32_t getCurrentMs ()
{
        static uint32_t i = 0;
        return ++i;
}

void transitionPrint (uint8_t name)
{
}

void transitionIndicate () {}

/**
 * @brief TEST_CASE
 */
TEST_CASE ("StringCondition test")
{
        StringCondition ok ("ala ma kota", StringCondition::DONT_STRIP);
        REQUIRE (!ok.checkImpl ("janek ma psa"));
        REQUIRE (ok.checkImpl ("ala ma kota"));
        REQUIRE (!ok.checkImpl (" ala ma kota"));
        REQUIRE (!ok.checkImpl ("ala ma kota "));
        REQUIRE (!ok.checkImpl (" ala ma kota "));
        REQUIRE (!ok.checkImpl ("ala ma kota\r\n"));

        StringCondition con ("ala ma kota", StringCondition::STRIP);

        REQUIRE (!con.checkImpl ("janek ma psa"));
        REQUIRE (con.checkImpl ("ala ma kota"));
        REQUIRE (con.checkImpl (" ala ma kota"));
        REQUIRE (con.checkImpl ("ala ma kota "));
        REQUIRE (con.checkImpl (" ala ma kota "));
        REQUIRE (con.checkImpl ("ala ma kota\r\n"));
}

/**
 * @brief TEST_CASE
 */
TEST_CASE ("StringCondition starts")
{
        BeginsWithCondition ok ("test", StringCondition::DONT_STRIP);
        bool b;

        REQUIRE (!ok.checkImpl ("janek ma psa"));
        REQUIRE (ok.checkImpl ("test ma kota"));
        REQUIRE (!ok.checkImpl (" test  ma kota"));
        REQUIRE (ok.checkImpl ("test ma kota "));
        REQUIRE (!ok.checkImpl (" test ma kota "));
        REQUIRE (ok.checkImpl ("test ma kota\r\n"));

        BeginsWithCondition con ("franio", StringCondition::STRIP);

        REQUIRE (!con.checkImpl ("janek ma psa"));
        REQUIRE (con.checkImpl ("franio ma kota"));
        REQUIRE (con.checkImpl (" franio ma kota"));
        REQUIRE (con.checkImpl ("franio ma kota "));
        REQUIRE (con.checkImpl (" franio ma kota "));
        REQUIRE (con.checkImpl ("franio ma kota\r\n"));
}

TEST_CASE ("Like condition wo stripping test")
{
        LikeCondition like1 ("+CSQ: 99:%:66");
        bool b;

        REQUIRE (like1.checkImpl ("+CSQ: 99:0:66")); // select '+CSQ: 99:0:66' like '+CSQ: 99:%:66'; => t
        REQUIRE (!like1.checkImpl ("+CSQ: 99:0:666"));
        REQUIRE (like1.checkImpl ("+CSQ: 99:2345:66"));
        REQUIRE (like1.checkImpl ("+CSQ: 99::66"));
        REQUIRE (like1.checkImpl ("+CSQ: 99:JHGHJ:88:66"));
        REQUIRE (like1.checkImpl ("+CSQ: 99:99:66"));
        REQUIRE (!like1.checkImpl ("+CSQ: 99:99#66"));
        REQUIRE (!like1.checkImpl ("+CSQ: 98:0:66"));
        REQUIRE (!like1.checkImpl ("CSQ: 99:0:66"));
        REQUIRE (!like1.checkImpl ("+CSQ: 98:0"));
        REQUIRE (!like1.checkImpl ("CSQ: 99:0"));
        REQUIRE (!like1.checkImpl ("+CSQ: 98"));
        REQUIRE (!like1.checkImpl ("CSQ: 99"));
        REQUIRE (!like1.checkImpl (""));

        // TODO ten test ma przechodzić
        // REQUIRE (like1.checkImpl ("+CSQ: 99::66:66")); // select '+CSQ: 99::66:66' like '+CSQ: 99:%:66';

        LikeCondition like2 ("+CSQ: 99:%");

        REQUIRE (like2.checkImpl ("+CSQ: 99:0"));
        REQUIRE (like2.checkImpl ("+CSQ: 99:2345"));
        REQUIRE (like2.checkImpl ("+CSQ: 99:"));
        REQUIRE (like2.checkImpl ("+CSQ: 99:JHGHJ:88"));
        REQUIRE (like2.checkImpl ("+CSQ: 99:99"));
        REQUIRE (!like2.checkImpl ("+CSQ: 98:0"));
        REQUIRE (!like2.checkImpl ("CSQ: 99:0"));

        LikeCondition like3 ("99:%:66|%|77");

        REQUIRE (like3.checkImpl ("99:bb:66|aa|77"));
        REQUIRE (like3.checkImpl ("99:b:66|a|77"));
        REQUIRE (like3.checkImpl ("99::66||77"));
        REQUIRE (like3.checkImpl ("99:bb:77:66|aa|78|77"));
        REQUIRE (!like3.checkImpl ("99:bb:77:66|aa|78|79"));

        //        REQUIRE (like3.checkImpl ("99::66||77"));

        LikeCondition like4 ("+CREG: %,0%");
        REQUIRE (like4.checkImpl ("+CREG: 1,0"));
        REQUIRE (like4.checkImpl ("+CREG: 0,0"));
        REQUIRE (like4.checkImpl ("+CREG: 0,0\r\n"));

        // TODO zaimplementować _
        //        LikeCondition like3 ("+CSQ:_99");

        LikeCondition like5 ("%41 0C%", StringCondition::STRIP, Condition::RETAIN_INPUT);

        REQUIRE (like5.checkImpl (">41 0C 00 40 "));
        REQUIRE (like5.checkImpl ("\r>41 0C 00 40 "));
        REQUIRE (like5.checkImpl ("\r\n>41 0C 00 40 "));
        REQUIRE (like5.checkImpl (">41 0C 00 40\r"));
        REQUIRE (like5.checkImpl (">41 0C 00 40\r\n"));
        REQUIRE (like5.checkImpl ("\r\n>41 0C 00 40\r\n"));

        REQUIRE (like5.checkImpl ("41 0C 00 40 "));
        REQUIRE (like5.checkImpl ("\r41 0C 00 40 "));
        REQUIRE (like5.checkImpl ("\r\n41 0C 00 40 "));
        REQUIRE (like5.checkImpl ("41 0C 00 40\r"));
        REQUIRE (like5.checkImpl ("41 0C 00 40\r\n"));
        REQUIRE (like5.checkImpl ("\r\n41 0C 00 40\r\n"));

        REQUIRE (like5.checkImpl (">41 0C 00 00 "));
        REQUIRE (like5.checkImpl ("\r>41 0C 00 00 "));
        REQUIRE (like5.checkImpl ("\r\n>41 0C 00 00 "));
        REQUIRE (like5.checkImpl (">41 0C 00 00\r"));
        REQUIRE (like5.checkImpl (">41 0C 00 00\r\n"));
        REQUIRE (like5.checkImpl ("\r\n>41 0C 00 00\r\n"));

        REQUIRE (like5.checkImpl ("41 0C 00 00 "));
        REQUIRE (like5.checkImpl ("\r41 0C 00 00 "));
        REQUIRE (like5.checkImpl ("\r\n41 0C 00 00 "));
        REQUIRE (like5.checkImpl ("41 0C 00 00\r"));
        REQUIRE (like5.checkImpl ("41 0C 00 00\r\n"));
        REQUIRE (like5.checkImpl ("\r\n41 0C 00 00\r\n"));
}

enum MyStates { INITIAL_STATE = 1, ALIVE_STATE, POWER_DOWN_STATE };

/**
 * @brief TEST_CASE
 */
TEST_CASE ("Pierwszy")
{
        gsmModemCommandsIssued.clear ();

        GsmCommandAction at ("AT");
        State initialState (INITIAL_STATE, &at);

        GsmCommandAction poweroff ("POWEROFF");
        GsmCommandAction blah ("BLAH");
        State aliveState (ALIVE_STATE, &poweroff, &blah);

        State powerDownState (POWER_DOWN_STATE);

        /*---------------------------------------------------------------------------*/

        GsmCommandAction xyz ("XYZ");
        StringCondition ok ("OK");
        Transition a (&ok, ALIVE_STATE, &xyz);
        initialState.addTransition (&a);

        Transition b (&ok, POWER_DOWN_STATE, &xyz);
        aliveState.addTransition (&b);

        /*---------------------------------------------------------------------------*/

        StringQueue inputQueue (STRING_QUEUE_SIZE);
        StateMachine machine (&inputQueue);

        machine.setInitialState (&initialState);
        machine.addState (&initialState);
        machine.addState (&aliveState);
        machine.addState (&powerDownState);

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
        REQUIRE (machine.currentState->getLabel () == INITIAL_STATE);

        // Symulujemy, że modem odpowiedział "OK", co pojawiło się na kolejce danych we. Maszyna
        // monitoruje tą kolejkę.
        inputQueue.push_back ();
        StringQueue::Element *el = inputQueue.back ();
        strcpy (el->data, "OK");

        // Trzecie uruchomienie maszyny, transition złapała odpowiedź "OK" i zadecydowała o zmianie stanu.
        machine.run ();

        // Akcje.
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);

        // Jeżeli maszyna stanów dokonała zmiany stanu, to automatycznie powinna zdjąć element z kolejki.
        //        REQUIRE (inputQueue.size () == 0);

        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");       // To jest pierwsza komenda, ktorą maszyna wysłała z entry action ze stanu initialState.
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ");      // To jest transition action.
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF"); // A to dodała entry action ze stanu aliveState

        /*---------------------------------------------------------------------------*/

        // Symulujemy znów odpowieddź od modemu, że OK
        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "OK");
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN_STATE);

        REQUIRE (gsmModemCommandsIssued.size () == 5);
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH"); // Z exit action stanu aliveState
        REQUIRE (gsmModemCommandsIssued[4] == "XYZ");  // Z transition action między alive a powerdown

        REQUIRE (inputQueue.size () == 0);
}

/**
 * @brief Testuje co się stanie jak na wejściu pojawi się dużo niepotrzebnego śmiecia.
 */
TEST_CASE ("Irrelevant input")
{
        gsmModemCommandsIssued.clear ();

        GsmCommandAction at ("AT");
        State initialState (INITIAL_STATE, &at);

        GsmCommandAction poweroff ("POWEROFF");
        GsmCommandAction blah ("BLAH");
        State aliveState (ALIVE_STATE, &poweroff, &blah);

        State powerDownState (POWER_DOWN_STATE);

        /*---------------------------------------------------------------------------*/

        GsmCommandAction xyz ("XYZ");
        StringCondition ok ("OK");
        Transition a (&ok, ALIVE_STATE, &xyz);
        initialState.addTransition (&a);

        Transition b (&ok, POWER_DOWN_STATE, &xyz);
        aliveState.addTransition (&b);

        /*---------------------------------------------------------------------------*/

        StringQueue inputQueue (STRING_QUEUE_SIZE);
        StateMachine machine (&inputQueue);

        machine.setInitialState (&initialState);
        machine.addState (&initialState);
        machine.addState (&aliveState);
        machine.addState (&powerDownState);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");
        REQUIRE (machine.currentState->getLabel () == INITIAL_STATE);

        /*---------------------------------------------------------------------------*/
        /* Drugie uruchomienie maszyny. Modem wysyła niepotrzebne dane.              */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        StringQueue::Element *el = inputQueue.back ();
        strcpy (el->data, "VERSION 123");

        machine.run ();
        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (machine.currentState->getLabel () == INITIAL_STATE);
        //        REQUIRE (inputQueue.size () == 0);

        /*---------------------------------------------------------------------------*/
        /* Drugie uruchomienie maszyny. Modem wysyła kilka linii do zignorowania.    */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "string1");

        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "string2");

        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "string3");

        //        REQUIRE (inputQueue.size () == 3);
        machine.run ();
        //        REQUIRE (inputQueue.size () == 2);
        machine.run ();
        //        REQUIRE (inputQueue.size () == 1);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (machine.currentState->getLabel () == INITIAL_STATE);
        //        REQUIRE (inputQueue.size () == 0);

        /*---------------------------------------------------------------------------*/
        /* Modem wysyła kilka linii do zignorowania, a na końcu OK                   */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "string1");

        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "string2");

        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "OK");

        // Tu się zmienia stan
        machine.run ();
        machine.run ();
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);

        // Tu się uruchamiają akcje
        machine.run ();
        //        REQUIRE (inputQueue.size () == 0);
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");       // To jest pierwsza komenda, ktorą maszyna wysłała z entry action ze stanu initialState.
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ");      // To jest transition action.
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
TEST_CASE ("Multiple relevant input")
{
        gsmModemCommandsIssued.clear ();

        GsmCommandAction at ("AT");
        State initialState (INITIAL_STATE, &at);

        GsmCommandAction poweroff ("POWEROFF");
        GsmCommandAction blah ("BLAH");
        State aliveState (ALIVE_STATE, &poweroff, &blah);

        State powerDownState (POWER_DOWN_STATE);

        /*---------------------------------------------------------------------------*/

        GsmCommandAction xyz ("XYZ");
        StringCondition ok ("OK");
        Transition a (&ok, ALIVE_STATE, &xyz);
        initialState.addTransition (&a);

        Transition b (&ok, POWER_DOWN_STATE, &xyz);
        aliveState.addTransition (&b);

        /*---------------------------------------------------------------------------*/

        StringQueue inputQueue (STRING_QUEUE_SIZE);
        StateMachine machine (&inputQueue);
        machine.addState (&initialState);
        machine.addState (&aliveState);
        machine.addState (&powerDownState);
        machine.setInitialState (&initialState);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        StringQueue::Element *el = inputQueue.back ();
        strcpy (el->data, "OK");

        // Irrelevant
        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "VERSION !@#");

        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "OK");

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL_STATE);

        /*
         * Wykona entryAction z initialState.entryAction (stąd jest "AT")
         * Pobierze z koleki wejściowej "OK"
         * Sprawdzi warunek i zmieni stan na aliveState
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);

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
        el = inputQueue.back ();
        strcpy (el->data, "OK");

        /*
         * Nie wykona żadnej akcji
         * Pobierze "OK" z kolejki
         * Znajdzie przejscie i zmieni stan na powerDownState
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN_STATE);

        /*
         * Wykona exitAction z aliveState
         * Wykona transition action.
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 5);
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH"); // Z exit action stanu aliveState
        REQUIRE (gsmModemCommandsIssued[4] == "XYZ");  // Z transition action między alive a powerdown
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN_STATE);
        REQUIRE (inputQueue.size () == 0);
}

/**
 * @brief Testuje opóźnienia.
 */
TEST_CASE ("Delays")
{
        gsmModemCommandsIssued.clear ();

        DelayAction delay (3);
        GsmCommandAction at ("AT");
        AndAction a1 (&delay, &at);

        State initialState (INITIAL_STATE, &a1);

        GsmCommandAction poweroff ("POWEROFF");
        GsmCommandAction blah ("BLAH");
        State aliveState (ALIVE_STATE, &poweroff, &blah);

        State powerDownState (POWER_DOWN_STATE);

        /*---------------------------------------------------------------------------*/

        DelayAction delay2 (3);
        GsmCommandAction xyz ("XYZ");
        AndAction a2 (&delay2, &xyz);


        StringCondition ok ("OK");
        Transition a (&ok, ALIVE_STATE, &a2);
        initialState.addTransition (&a);

        Transition b (&ok, POWER_DOWN_STATE, &a2);
        aliveState.addTransition (&b);

        /*---------------------------------------------------------------------------*/

        StringQueue inputQueue (STRING_QUEUE_SIZE);
        StateMachine machine (&inputQueue);
        machine.addState (&initialState);
        machine.addState (&aliveState);
        machine.addState (&powerDownState);
        machine.setInitialState (&initialState);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        inputQueue.push_back ();
        StringQueue::Element *el = inputQueue.back ();
        strcpy (el->data, "OK");

        // Irrelevant
        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "VERSION !@#");

        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "OK");

        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL_STATE);
        REQUIRE (gsmModemCommandsIssued.size () == 0);
        //        REQUIRE (inputQueue.size () == 3);

        machine.run ();
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 0);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "AT");
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (gsmModemCommandsIssued[0] == "AT");
        REQUIRE (gsmModemCommandsIssued[1] == "XYZ");
        REQUIRE (gsmModemCommandsIssued[2] == "POWEROFF");

        // Kolejka jest czyszcona zgodnie z nowym sposobem po każdym przejsicu stanowym, które go warunki czytały (konsumowały) wejście.
        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "OK");

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 3);
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN_STATE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 4); // 4 bo było wyjście z alive, i exit Action się wykonało
        REQUIRE (gsmModemCommandsIssued[3] == "BLAH"); // Z exit action stanu aliveState
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 4);
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 5);
        REQUIRE (gsmModemCommandsIssued[4] == "XYZ"); // Z transition action między alive a powerdown
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN_STATE);
        REQUIRE (inputQueue.size () == 0);
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
TEST_CASE ("Non input condition")
{
        gsmModemCommandsIssued.clear ();

        GsmCommandAction at ("INITIAL ENTRY");
        State initialState (INITIAL_STATE, &at);

        GsmCommandAction poweroff ("ALIVE ENTRY");
        State aliveState (ALIVE_STATE, &poweroff);

        State powerDownState (POWER_DOWN_STATE);

        /*---------------------------------------------------------------------------*/

        bool cond = false;
        BoolCondition fakeCond (&cond);
        Transition a (&fakeCond, ALIVE_STATE);
        initialState.addTransition (&a);

        StringCondition ok ("OK");
        Transition b (&ok, POWER_DOWN_STATE);
        aliveState.addTransition (&b);

        /*---------------------------------------------------------------------------*/

        StringQueue inputQueue (STRING_QUEUE_SIZE);
        StateMachine machine (&inputQueue);
        machine.addState (&initialState);
        machine.addState (&aliveState);
        machine.addState (&powerDownState);
        machine.setInitialState (&initialState);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Irrelevant
        inputQueue.push_back ();
        StringQueue::Element *el = inputQueue.back ();
        strcpy (el->data, "VERSION !@#");

        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "OK");

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL_STATE);
        REQUIRE (gsmModemCommandsIssued.size () == 0);

        /*
         * Wykona entryAction z initialState.entryAction
         * Pobierze z koleki wejściowej "VERSION !@#"
         * Sprawdzi warunek fake i wyjdzie
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "INITIAL ENTRY");
        REQUIRE (machine.currentState->getLabel () == INITIAL_STATE);

        // Nic się nie stanie.
        // W kolejce wejściowej cały czas "VERSION !@#"
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "INITIAL ENTRY");
        REQUIRE (machine.currentState->getLabel () == INITIAL_STATE);

        /*
         * Sprawdzi warunek fake i zmieni stan na aliveState
         * W kolejce wejściowej cały czas "VERSION !@#"
         */
        cond = true;
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "INITIAL ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);

        /*
         * Wykona akcję entry action z aliveState
         * Pobierze z koleki wejściowej "VERSION !@#"
         * Żaden warunek nie przejdzie, ale skonsumuje input, więc wyczyści kolejkę
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 2);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");
        REQUIRE (gsmModemCommandsIssued[1] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);

        // Kolejka jest czyszcona zgodnie z nowym sposobem po każdym przejsicu stanowym, które go warunki czytały (konsumowały) wejście.
        inputQueue.push_back ();
        el = inputQueue.back ();
        strcpy (el->data, "OK");

        /*
         * Nie wykona akcji.
         * Pobierze z kolejki "OK"
         * Warunek się spełni, przejdzie do powerDownState
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 2);
        REQUIRE (machine.currentState->getLabel () == POWER_DOWN_STATE);
}

/**
 * @brief Testuje czy działa przejśącie stanowe do samego siebie i czy za każdym razem wykonują się wszystkie akcje.
 */
TEST_CASE ("Transition to myself")
{
        gsmModemCommandsIssued.clear ();

        GsmCommandAction at ("INITIAL ENTRY");
        State initialState (INITIAL_STATE, &at);

        GsmCommandAction entry ("ALIVE ENTRY");
        GsmCommandAction exit ("ALIVE EXIT");
        State aliveState (ALIVE_STATE, &entry, &exit);

        /*---------------------------------------------------------------------------*/

        static bool bbb = true;
        BoolCondition fakeCond (&bbb);
        Transition a (&fakeCond, ALIVE_STATE);
        initialState.addTransition(&a);

        Transition b (&fakeCond, ALIVE_STATE);
        aliveState.addTransition(&b);

        /*---------------------------------------------------------------------------*/

        StringQueue inputQueue (STRING_QUEUE_SIZE);
        StateMachine machine (&inputQueue);
        machine.addState (&initialState);
        machine.addState (&aliveState);
        machine.setInitialState (&initialState);

        /*---------------------------------------------------------------------------*/
        /* Uruchamiamy urządzenie                                                    */
        /*---------------------------------------------------------------------------*/

        // Przejdzie to initial State (initial Transition).
        machine.run ();
        REQUIRE (machine.currentState->getLabel () == INITIAL_STATE);
        REQUIRE (gsmModemCommandsIssued.size () == 0);

        /*
         * Wykona entryAction z initialState.entryAction
         * Pobierze z koleki wejściowej "VERSION !@#"
         * Sprawdzi warunek fake i wyjdzie
         */
        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 1);
        REQUIRE (gsmModemCommandsIssued.back () == "INITIAL ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 2);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");
        REQUIRE (gsmModemCommandsIssued[1] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 4);
        REQUIRE (gsmModemCommandsIssued[0] == "INITIAL ENTRY");
        REQUIRE (gsmModemCommandsIssued[1] == "ALIVE ENTRY");
        REQUIRE (gsmModemCommandsIssued[2] == "ALIVE EXIT");
        REQUIRE (gsmModemCommandsIssued[3] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 6);
        REQUIRE (gsmModemCommandsIssued[4] == "ALIVE EXIT");
        REQUIRE (gsmModemCommandsIssued[5] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);

        machine.run ();
        REQUIRE (gsmModemCommandsIssued.size () == 8);
        REQUIRE (gsmModemCommandsIssued[6] == "ALIVE EXIT");
        REQUIRE (gsmModemCommandsIssued[7] == "ALIVE ENTRY");
        REQUIRE (machine.currentState->getLabel () == ALIVE_STATE);
}
