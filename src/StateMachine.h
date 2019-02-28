#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "State.h"
#include "StateMachineTypes.h"
#include "StringQueue.h"
#include "TimeCounter.h"
#include "Transition.h"
#include <cstring>

// TODO
// namespace sm {
/**
 * Do synchronizacji 2 maszyn. Uwaga, tylko potęgi dwójki.
 */
#define SYNCHRO_MODULO 2

/**
 * @brief The StateMachine class
 * TODO Dodać globalny timeout. Jeśli w danym timeoucie nie uda się zmienić stanu, to trzeba podjąć jakąć
 * akcję. Ej to chyba jest!
 *
 * TODO dokumentacja z tutorialem jest niezbędna, bo ja sam mam czasem problemy. Jak są ewaluowane warunki,
 * co do nich wpada, w jakiej kolejności i tak dalej. Opisać wszystkie aspekty działania : jak dwie maszyny
 * mogą pracować na jedym input/output i tak dalej.
 *
 * TODO opisać, że zawsze trzeba pamiętyać czy jest odpowiednia ilość czasu na sprawdzenie warunków. Podać taki przykład:
 * Kiedy jest jeden warunek na przejście, który oczekuje jakichś danych, to nie ma problemu. Na przykład :
 *
 *         m->state (INIT, State::INITIAL)->entry (at ("ATE1\r\n"))
 *               ->transition (CHECK_OPERATING_MODE)->when (&ok)->then (&delay);
 *
 * Maszyna będzie tak długo się kręciuć, aż dostanie "OK". Jeżeli jednak mamy warunki zdefiniowane tak, że oczekiwane wejście
 * może nie zdążyć się pojawić, to trzeba dodać opóźnienie, żeby poczekać na te dane wejściowe. Przykład (błędny):
 *
 *         m->state (CHECK_OPERATING_MODE)->entry (at ("AT+CWMODE_DEF?\r\n"))
 *               ->transition (VERIFY_CONNECTED_ACCESS_POINT)->when (eq ("+CWMODE_DEF:1"))->then (&delay)
 *               ->transition (SET_OPERATING_MODE)->when (&alwaysTrue)->then (&delay);
 *
 * W powyższym przykładzie warunki dwóch przejść są sprawdzane jedno po drugim. Najpierw maszyna sprawdzi, czy w kolejce
 * wejściowej jest jakiś element z napisem "+CWMODE_DEF:1", a jeśli nie ma, to natychmiast przejdzie do stanu SET_OPERATING_MODE.
 * Trzeba dać maszynie trochę czasu na oczekiwanie na dane. Powinno być:
 *
 *         m->state (CHECK_OPERATING_MODE)->entry (and_action (at ("AT+CWMODE_DEF?\r\n"), &delay))
 *               ->transition (VERIFY_CONNECTED_ACCESS_POINT)->when (eq ("+CWMODE_DEF:1"))->then (&delay)
 *               ->transition (SET_OPERATING_MODE)->when (&alwaysTrue)->then (&delay);
 *
 * Taki błąd jak wyżej może prowadzić do trudnych do wykrycia konsekwencji. Jeśli jak wyżej mamy dwa przejścia stanowe, to
 * dość łatwo będzie się zorientować, że mimo że modem wysłał odpowiedź "+CWMODE_DEF:1" to jesteśmy w słym stanie (w SET_OPERATING_MODE
 * zamiast VERIFY_CONNECTED_ACCESS_POINT). Ale w przypadku gdy mamy takie przejście (błędne - brakuje delay):
 *
 *         m->state (CLOSE_AND_RECONNECT)->entry (at ("AT+CIPCLOSE=0\r\n"))
 *               ->transition (CONNECT_TO_SERVER)->when(&alwaysTrue)->then (&delay);
 *
 * To w efekcie maszyna przejdzie prawidłowo do stanu CONNECT_TO_SERVER, ale *za szybko* i może złapać jescze dane wejściowe
 * które jej nie interesują i które spowodują dalsze nieprawidłowe przejścia.
 *
 * TODO Ten problem pojawia się częściej niż mi się wydawało rozwiązaniem jest :
 * - Po każdym żądaniu do modemu powinniśmy odczekać chwilkę, bo przecież odpowiedzi
 * może być więcej niż 1 i przychodzą w pewnym okresie czasu (po wysłaniu żądania,
 * ale jest jednak pewne opóźniene). Może akcja at niech się łączy z akcją delay - przydał
 * by się "szablon"!
 * - Wrunki przejść powinniśmy konstruować tak, żeby wymagały wszystkich możliwych
 * odpowiedzi z modemu, czyli trzba oczekiwac na echo, na jakieś tam dane i na OK.
 * - TODO Oczekiwać odpowiedzi w odpowiedniej kolejności - czytaj następne TODO.
 * Błędy niestety są trudne do wykrycia. Przykład z życia : miałem takie przejścia:
 *
 *         m->state (PDP_CONTEXT_CHECK)->entry (at ("AT+QIACT?\r\n"))
 *               ->transition(DNS_CONFIG)->when (like ("+QIACT:%1,%"))->then (&delay)
 *               ->transition(ACTIVATE_PDP_CONTEXT)->when (anded (beginsWith ("AT+QIACT?"), &ok))->then (&delay);
 *
 * Tu mamy odpowiedź z modemu :
 * IN : AT+QIACT?
 * IN : +QIACT: 1,1,1,"37.249.238.180"
 * IN : OK
 *
 * Uwaga! Są 3 odpowiedzi, ale już po drugiej warunek przejścia do DNS_CONFIG jest spełniony i wykonuje się przejście,
 * następnie przychodzi odpowiedź OK co oznacza, że w kolejce zostaje OK i ono trafi jako wejście w następnym stanie
 * (innymi słowy stan ACTIVATE_PDP_CONTEXT będzie myślał, że już ma odpowiedź OK).
 *
 *       m->state (ACTIVATE_PDP_CONTEXT)->entry (at ("AT+QIACT=1\r\n"))
 *               ->transition(DNS_CONFIG)->when (anded (beginsWith ("AT+QIACT="), &ok))->then (&delay);
 *
 * IN : AT+QIACT=1
 * IN : ERROR
 *
 * Czyli otrzymaliśmy błąd! Żadne przejście nie powinno sie wykonać. Ale niestety w kolejce zostało OK, więc się wykonuje
 * przejście do DNS_CONFIG. A następnie przychodzi odpowiedź ERROR.
 *
 *       m->state (DNS_CONFIG)->entry (at ("AT+QIDNSCFG=1,\"8.8.8.8\",\"8.8.4.4\"\r\n"))
 *               ->transition (INIT)->when (&error)->then (&longDelay)
 *               ->transition (NETWORK_GPS_USART_ECHO_OFF)->when (anded (beginsWith ("AT+QIDNSCFG="), &ok))->then (&delay);
 *
 * No tu jest kompletna klapa, bo idzie do init, bo ma error.
 *
 * TODO Powyższy problem jest poważny, bo kolejny raz z nim walczę. Problem by się (częściowo?) rozwiązał gdyby
 * warunki były spełnione, tylko gdy odpowiedzi przyjdą w odpowiedniej kolejności.
 *
 * TODO Zastanowić się, czy dałoby się *łatwo* uniezaleznić tę implementację od StringQueue. Jeśli tak, to
 * implementacja byłaby bardziej uniwersalna.
 *
 * TODO Powstał problem odbierania danych binarnych. Ja zaszyłem w maszynie funkcjonalność operacji na zero-ended
 * strings i przez to nie mam teraz jak obsłużyć danych binarnych. Jeszcze gorzej, że zaszyłem na stałe uzywanie
 * konkretnej kolekcji StringQueue, której elementy mają zawsze 128B W przypadku próby odbierania większej ilości
 * danych to będzie problem (przydał by się CircularBuffer). Natomiast w przypadku próby odbierania całego firmware,
 * to już w ogóle będzie masakra (bo wtedy MySink musiałby kopiowac dane do jakiegoś mega bufora, albo wręcz nagrywać
 * je bezpośrednio na flash).
 *
 * TODO Pousuwać konstruktyory i metody, których nie używam w ogóle.
 *
 * TODO Zależność od libmicro dać jako opcję (ifdef)
 */
template <typename EventT = string> class StateMachine {
public:
        using EventType = EventT;
        using Types = StateMachineTypes<EventType>;
        using EventQueue = typename Types::EventQueue;
        using ActionType = Action<EventType>;
        using ActionQueue = Queue<ActionType *>; // Typ kolejki akcji. Maszyna odkłada na taką kolejkę akcje entry, exit i transition.
        using StateType = State<EventType>;
        using ConditionType = Condition<EventType>;
        using TransitionType = Transition<EventType>;

        StateMachine (uint32_t logId = 0, bool useOnlyOneInputAtATime = false)
            : lastAddedState (nullptr),
              lastAddedTransition (nullptr),
              lastAddedTransitionRF (nullptr),
              lastAddedTransitionRL (nullptr),
              lastAddedAction (nullptr),
              initialState (nullptr),
              currentState (nullptr),
              firstTransitionRL (nullptr),
              firstTransitionRF (nullptr),
              eventQueue (EVENT_QUEUE_SIZE),
              actionQueue (ACTION_QUEUE_SIZE),
              logId (logId),
              synchroCounter (nullptr),
              useOnlyOneInputAtATime (useOnlyOneInputAtATime)
        {
                inputCopy[0] = '\0';
                memset (states, 0, sizeof (states));
        }

        /**
         * @brief Fires the state machine's main loop.
         * Sprawdza kolejki z danymi wejściowymi i/lub różne opóźnienia. Nawet jeżeli nie ma danych
         * wejściowych, maszyna może podjąć jakąś akcję na podstaiwe opóźnień (długi brak danych we. etc).
         */
        void run ();

        /**
         * @brief Resetuje maszynę.
         * Przełącza maszynę w stan initialState (bez uruchamiania akcji wyjściowych z poprzedniego stanu i bez
         * uruchomienia żadnych akcji transition). Czyści kolejkę. Maszyna zaczyna działać tak jak od pierwszego
         * uruchomienia. Przed wywołaniem reset można zmienić initialState za pomocą setInitialState(), ale nie
         * trzeba.
         */
        void reset ();
        uint8_t getCurrentStateLabel () const;

        void setSynchro (uint8_t *synchroCounter, uint8_t synchroModulo)
        {
                this->synchroCounter = synchroCounter;
                this->synchroModulo = synchroModulo;
        }

        void setTimeCounter (TimeCounter *value) { timeCounter = value; }

        StateMachine *state (uint8_t label, StateFlags flags = StateFlags::NONE); /// Nowy stan o nazwie label.
        StateMachine *entry (ActionType *action);                                 /// Entry action do ostatnio dodanego stanu.
        StateMachine *exit (ActionType *action);                                  /// Exit action do ostatnio dodanego stanu.
        StateMachine *transition (uint8_t to,
                                  TransitionPriority run
                                  = TransitionPriority::RUN_LAST); /// Przejście z ostatnio dodanego stanu do stanu o nazwie "to".

        StateMachine *when (ConditionType *cond); /// Warunek do ostatnio dodanego przejścia (transition).
        //        template <typename Func> StateMachine *whenf (Func func) { return when (new FuncCondition<Func> (func)); }

        StateMachine *then (ActionType *action); /// Akcja do ostatnio dodanego przejścia (transition).
        //        template <typename Func> StateMachine *thenf (Func func) { return then (new FuncAction<Func> (func)); }

        EventQueue &getEventQueue () { return eventQueue; }
        EventQueue const &getEventQueue () const { return eventQueue; }

        void setInitialState (StateType *s);
        void setInitialState (uint8_t stateLabel);
        void addState (StateType *s);
        void addGlobalTransition (TransitionType *t, TransitionPriority run = TransitionPriority::RUN_LAST);

private:
        bool check (ConditionType &condition, uint8_t inputNum, EventType &retainedInput);

private:
        StateType *lastAddedState;
        TransitionType *lastAddedTransition;
        TransitionType *lastAddedTransitionRF;
        TransitionType *lastAddedTransitionRL;
        ActionType *lastAddedAction;

#ifndef UNIT_TEST
private:
#else
public:
#endif

        void pushBackAction (ActionType *a);
        bool fireActions ();
        bool fixCurrentState ();
        TransitionType *findTransition ();
        void performTransition (TransitionType *t);

        enum Error {
                INPUT_QUEUE_EMPTY,
                OPERATION_QUEUE_FULL,
                TRANSITION_ARRAY_FULL,
                STATES_ARRAY_FULL,
                NO_LAST_ADDED_STATE,
                NO_LAST_ADDED_ACTION,
                NO_LAST_ADDED_TRANSITION,
                NO_SUCH_STATE
        };

        void errorCondition (Error e);

#ifndef UNIT_TEST
private:
#endif

        StateType *initialState;
        StateType *currentState;
        TransitionType *firstTransitionRL; /// Run last
        TransitionType *firstTransitionRF; /// Run First
        EventQueue eventQueue;
        StateType *states[MAX_STATES_NUM];
        // Z kolejki jest kopiowane tutaj kiedy warunek zostanie spełniony i ma ustawione RETAIN
        EventType inputCopy;
        ActionQueue actionQueue;
        TimeCounter *timeCounter = nullptr;
        uint32_t logId;
        uint8_t *synchroCounter;
        uint8_t synchroModulo;

        /**
         * Kiedy true, to maszyna pracuje na jednym wejściu na raz. To oznacza, że warunki są sprawdzane tylko na jednym wejściu,
         * i potem jak przejscie zoatsnie znależione, to tylko ot jednojest zdejmowane. Jeśli to pole jest false, to pracuje na
         * całej kolejce (może być w niej wiele wejść). Jeśli znajdzie przejscie, to czyści całą kolejkę wejściową po znalezieniu
         * przejścia.
         * Domyślnie false
         */
        bool useOnlyOneInputAtATime;
};

/*****************************************************************************/

template <typename EventT> bool StateMachine<EventT>::fireActions ()
{
        while (actionQueue.size ()) {
                ActionType *op = actionQueue.front ();
                bool finished = op->run (inputCopy);

                if (!finished) {
                        return false;
                }
                else {
                        actionQueue.pop_front ();
                }
        }

        return true;
}

/*****************************************************************************/

template <typename EventT> bool StateMachine<EventT>::fixCurrentState ()
{
        if (!currentState) {
                currentState = initialState;

#ifndef UNIT_TEST
                uint8_t currentLabel = currentState->getLabel ();
                debug->print ("fixCurrentState : ");
                debug->println (currentLabel);
#endif

                if (timeCounter) {
                        timeCounter->set ();
                }

                pushBackAction (currentState->getEntryAction ());
                return false;
        }

        return true;
}

/*****************************************************************************/

template <typename EventT> bool StateMachine<EventT>::check (ConditionType &condition, uint8_t inputNum, EventType &retainedInput)
{
        /*
         * Conditions are stateful i.e. they remember the result of last check,
         * so here we reset their state.
         */
        condition.reset ();

        if (eventQueue.size ()) {

                /*
                 * If state wouldn't be persisted, then
                 */
                for (int i = 0; i < inputNum; ++i) {
                        if (condition.check (eventQueue.front (i), retainedInput)) {
                                break;
                        }
                }
        }
        else {
                condition.check (EventType (), retainedInput);
        }

        return condition.getResult ();
}

/*****************************************************************************/

template <typename EventT> typename StateMachine<EventT>::TransitionType *StateMachine<EventT>::findTransition ()
{
        TransitionType *ret = nullptr;
        enum Stage { FIRSTS, REGULARS, LASTS };
        Stage stage;
        TransitionType *t = nullptr;
        bool suppressGlobalTransitions
                = (currentState->getFlags () & StateFlags::SUPPRESS_GLOBAL_TRANSITIONS) == StateFlags::SUPPRESS_GLOBAL_TRANSITIONS;

        if (suppressGlobalTransitions) {
                stage = REGULARS;
                t = currentState->getFirstTransition ();
        }
        else {
                stage = FIRSTS;
                t = firstTransitionRF;
        }

#ifndef UNIT_TEST
        __disable_irq ();
#endif

        uint8_t noOfInputs = (useOnlyOneInputAtATime) ? (1) : (eventQueue.size ());

#ifndef UNIT_TEST
        __enable_irq ();
#endif

        while (1) {

                if (!t || !t->getCondition () || !check (*t->getCondition (), noOfInputs, inputCopy)) {

                        if (t) {
                                t = t->next;
                        }

                        if (!t) {
                                if (stage == FIRSTS) {
                                        t = currentState->getFirstTransition ();
                                        stage = REGULARS;
                                        continue;
                                }
                                else if (stage == REGULARS && !suppressGlobalTransitions) {
                                        t = firstTransitionRL;
                                        stage = LASTS;
                                        continue;
                                }
                                else {
                                        break;
                                }
                        }
                }
                else {
                        ret = t;
                        break;
                }
        }

#ifndef UNIT_TEST
        __disable_irq ();
#endif

        //        for (int i = 0; i < noOfInputs; ++i) {
        //                // I call "from_isr" version because I lock by myself.
        //                eventQueue.pop_from_isr ();
        //        }

        eventQueue.pop_front (noOfInputs);

#ifndef UNIT_TEST
        __enable_irq ();
#endif

        return ret;
}

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::performTransition (TransitionType *t)
{
        // Jeżeli spełniony, to przejdź wykonując akcje:
        // - exit dla aktualnego
        pushBackAction (currentState->getExitAction ());
        // - transition dla przejscia
        pushBackAction (t->getAction ());
        // - Entry.
        currentState = states[t->getTo ()];

        if (!currentState) {
                errorCondition (NO_SUCH_STATE);
        }

#ifndef UNIT_TEST
        uint8_t currentLabel = currentState->getLabel ();
        debug->print ("transition : ");
        debug->println (currentLabel);
#endif

        if (timeCounter) {
                timeCounter->set ();
        }

        pushBackAction (currentState->getEntryAction ());
#if 1
//        debugLedToggle ();
#endif
}

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::run ()
{
        if (synchroCounter && *synchroCounter % SYNCHRO_MODULO != synchroModulo) {
                return;
        }

        // Najpierw uruchom wszystkie akcje
        if (!fireActions ()) {
                return;
        }

        /*
         * Wykonaj initialTransition. Ona nie powoduje zdjęcia z kolejki wejściowej, bo
         * dokonuje się automatycznie, bez względu na dane wejściowe (w przeciwieństwie
         * do każdej innej tranzycji).
         */
        if (!fixCurrentState ()) {
                return;
        }

        TransitionType *t = findTransition ();

        if (!t) {
                return;
        }

        performTransition (t);

        if (currentState && ((currentState->getFlags () & StateFlags::INC_SYNCHRO) == StateFlags::INC_SYNCHRO)) {
                // printf ("pre inc SC = %d, SM = %d\n", (int)*synchroCounter, (int)synchroModulo);
                ++*synchroCounter;
        }

        /*
         * Uwaga, tu kolejka input powinna być wyczyszczona przez sprawdzanie warunków przejść powyżej.
         * Jeżeli nie jest, to znaczy, że warunki tak zostały skonstruowane, że coś zostaje (czyli zaakceptowano
         * wejście z początku, nie sprawdzając pozostałych). Jeżeli to nie było zamieżone, to należy przerobić
         * warunki tak, żeby obejmowały wszystkie przypadki (można użyć anded i ored).
         */
}

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::setInitialState (StateType *s) { initialState = s; }

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::setInitialState (uint8_t stateLabel) { initialState = states[stateLabel]; }

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::addGlobalTransition (TransitionType *t, TransitionPriority run)
{
        if (run == TransitionPriority::RUN_LAST) {
                if (!lastAddedTransitionRL) {
                        firstTransitionRL = lastAddedTransitionRL = lastAddedTransition = t;
                }
                else {
                        lastAddedTransitionRL->next = t;
                        lastAddedTransitionRL = lastAddedTransition = t;
                }
        }
        else {
                if (!lastAddedTransitionRF) {
                        firstTransitionRF = lastAddedTransitionRF = lastAddedTransition = t;
                }
                else {
                        lastAddedTransitionRF->next = t;
                        lastAddedTransitionRF = lastAddedTransition = t;
                }
        }
}

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::addState (StateType *s)
{
        if (s->getLabel () >= MAX_STATES_NUM) {
                errorCondition (STATES_ARRAY_FULL);
        }

        states[s->getLabel ()] = s;
}

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::pushBackAction (ActionType *a)
{
        if (!a) {
                return;
        }

        //        do {
        //                pushBackOperation (a);
        if (!actionQueue.push_back ()) {
                errorCondition (OPERATION_QUEUE_FULL);
        }

        typename ActionQueue::Element &el = actionQueue.back ();
        el = a;
        //        } while ((a = a->getNext ()));
}

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::errorCondition (Error e)
{
        while (true) {
        }
}

/*****************************************************************************/

template <typename EventT> StateMachine<EventT> *StateMachine<EventT>::state (uint8_t label, StateFlags flags)
{
        lastAddedState = new StateType (label);
        lastAddedState->setFlags (flags);
        addState (lastAddedState);

        if ((flags & StateFlags::INITIAL) == StateFlags::INITIAL) {
                setInitialState (lastAddedState);
        }

        return this;
}

/*****************************************************************************/

template <typename EventT> StateMachine<EventT> *StateMachine<EventT>::entry (ActionType *action)
{
        if (!lastAddedState) {
                errorCondition (NO_LAST_ADDED_STATE);
        }

        lastAddedAction = action;
        lastAddedState->setEntryAction (action);
        return this;
}

/*****************************************************************************/

template <typename EventT> StateMachine<EventT> *StateMachine<EventT>::exit (ActionType *action)
{
        if (!lastAddedState) {
                errorCondition (NO_LAST_ADDED_STATE);
        }

        lastAddedAction = action;
        lastAddedState->setExitAction (action);
        return this;
}

/*****************************************************************************/

// StateMachine *StateMachine<EventT>::and_action (ActionType *action)
//{
//        if (!lastAddedAction) {
//                errorCondition (NO_LAST_ADDED_ACTION);
//        }

//        lastAddedAction->setNext (action);
//        lastAddedAction = action;
//        return this;
//}

/*****************************************************************************/

template <typename EventT> StateMachine<EventT> *StateMachine<EventT>::transition (uint8_t to, TransitionPriority run)
{
        if (!lastAddedState) {
                // errorCondition (NO_LAST_ADDED_STATE);
                addGlobalTransition (new TransitionType (nullptr, to), run);
        }
        else {
                lastAddedTransition = new TransitionType (nullptr, to);
                lastAddedState->addTransition (lastAddedTransition);
        }
        return this;
}

/*****************************************************************************/

template <typename EventT> StateMachine<EventT> *StateMachine<EventT>::when (ConditionType *cond)
{
        if (!lastAddedTransition) {
                errorCondition (NO_LAST_ADDED_TRANSITION);
        }

        lastAddedTransition->setCondition (cond);
        return this;
}

/*****************************************************************************/

template <typename EventT> StateMachine<EventT> *StateMachine<EventT>::then (ActionType *action)
{
        if (!lastAddedTransition) {
                errorCondition (NO_LAST_ADDED_TRANSITION);
        }

        lastAddedTransition->setAction (action);
        lastAddedAction = action;
        return this;
}

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::reset (/*uint8_t state*/)
{
#ifndef UNIT_TEST
        __disable_irq ();
#endif

        eventQueue.clear ();

#ifndef UNIT_TEST
        __enable_irq ();
#endif

        currentState = nullptr;

        // Przez to TimeCountery wariowały. Ticket #83
        // if (timeCounter) {
        //         timeCounter->reset ();
        // }

        inputCopy[0] = '\0';
        actionQueue.clear ();
}

/*****************************************************************************/

template <typename EventT> uint8_t StateMachine<EventT>::getCurrentStateLabel () const
{
        if (currentState) {
                return currentState->getLabel ();
        }
        else {
                return 0;
        }
}

/*
 * Fixed bugs:
 *
 * TODO OrCondition i w ogóle API Condition ma poważny błąd. Usecase, żeby powtórzyć: Warunek:
 *
 * m->state (CONNECT_TO_SERVER)->entry (at ("AT+CIPSTART=0,\"TCP\",\"trackmatevm.cloudapp.net\",1883\r\n"))
 *     ->transition (NETWORK_ECHO_OFF)->when (ored (anded (like ("%,CONNECT"), &ok), eq ("CONNECT OK")))->then (&delay);
 *
 * Przychodzą dane i kolejka wejściowa zawiera następujące dane:
 * - OK
 * - 0,CONNECT
 * - CIPSTART cośtam
 *
 * Maszyna nie przechodzi do stanu NETWORK_ECHO_OFF. Problem polega na tym, że OrCondition wywołuje medodę checkAndRetain
 * swoich dwóch warunków, a w przypadku AndCondition checkAndRetain nie robi NIC! Prawdziwa implementacja jest w przedefiniowanycm
 * check. Moim zdaniem checkImpl i checkAndRetain powinny być prywatne, a jedynym słusznym api powinno być check.
 *
 * TODO Dodatkowo do tego co wyżej, to trzeba zrobić unit test warunku anded (a, anded (b, c)), bo mi się, wydaje, że to
 * nie zadziała!
 *
 * TODO checkImpl i checkRetained powinny być prywatne i nie powinno być friendów moim zdaniem. Rekurencyjne
 * warunki jak And i Or powinny jakoś używać głownej metody check. Tylko wtedy będzie problem z sekwencją.
 *
 */

//}

#endif // STATEMACHINE_H
