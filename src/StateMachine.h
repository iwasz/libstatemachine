/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef LIB_STATEMACHINE_H
#define LIB_STATEMACHINE_H

// Remember to add *all* new headers here. This header is a common one.
#include "Action.h"
#include "AndCondition.h"
#include "BeginsWithCondition.h"
#include "BoolAction.h"
#include "BoolCondition.h"
#include "Condition.h"
#include "Debug.h"
#include "DelayAction.h"
#include "IntegerAction.h"
#include "IntegerCondition.h"
#include "LikeCondition.h"
#include "NotCondition.h"
#include "OrCondition.h"
#include "SequenceCondition.h"
#include "State.h"
#include "StateChangeAction.h"
#include "StateCondition.h"
#include "StateMachineTypes.h"
#include "StringCondition.h"
#include "TimeCounter.h"
#include "TimePassedCondition.h"
#include "Transition.h"
#include <algorithm>
#include <cstring>
#include <etl/list.h>
#include <etl/queue.h>

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
 * TODO Zależność od libmicro dać jako opcję (ifdef) EDIT : sporo rzeczy nie będzie działać, ale trudno.
 *
 * TODO W walgrindzie jak się uruchomi unitytest to jeden nie przechodzi.
 *
 * TODO Trzeba opisac jak działa NotCondition, bo to nieintuicyjnie działa gdy jest więcej niż 1 odpowiedź (? przykład ?).
 *
 * TODO Easier interface for constructing the machine and for lambdas
 *
 * TODO Event arguments would resolve the problem of passing data between actions in elegant way and
 * would allow more flexible condition checks.
 *
 * TODO It would be fantastic if transitions, entry and exit actions were fired in
 * natural order that is entry , transition, transition action, exit. Now all actions
 * are run all at once.
 *
 * TODO Deferred event implenentation is shitty.
 *
 * TODO strasznie ciężko jest się połapać co się dzieje kiedy maszyna źle działa. Trzebaby poprawić debugi
 * tak, żeby pokazywały wejścia, zamiany stanów, wywołania akcji I WARUNKI we własciwej kolejności!
 *
 * TODO Żeby dało się robić transition bez warunków (co by działo jak alwaysTrue).
 *
 * TODO żeby do eventów moża było bez problemu dodawać dowlną liczbę argumentów o dowolnych typach.

TODO żeby nazwy stanów nie musiałby być definiowane osobno w enumie, i żeby łatwo się je dało
 * wypisać na debug. Może user-defined literals?
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class StateMachine {
public:
        using EventType = EventT;
        using Types = StateMachineTypes<EventType>;
        using EventQueue = typename Types::EventQueue;
        using DeferredEventContainer = etl::list<EventType, DEFERRED_EVENT_QUEUE_SIZE>;
        using ActionType = Action<EventType>;
        using ActionQueue = etl::queue<ActionType *, ACTION_QUEUE_SIZE,
                                       etl::memory_model::MEMORY_MODEL_SMALL>; // Typ kolejki akcji. Maszyna odkłada na taką kolejkę akcje entry,
                                                                               // exit i transition.
        using StateType = State<EventType>;
        using ConditionType = Condition<EventType>;
        using TransitionType = Transition<EventType>;
        using RuleType = Rule<EventType>;

        enum class Log { NO, YES };
        static constexpr size_t MAX_LOG_LINE_SIZE = 30;

        StateMachine (uint32_t logId = 0, bool useOnlyOneInputAtATime = false, Log log = Log::YES)
            : logId (logId), useOnlyOneInputAtATime (useOnlyOneInputAtATime), log (log)
        {
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

        StateMachine *rule (); /// Przejście z ostatnio dodanego stanu do stanu o nazwie "to".

        StateMachine *when (ConditionType *cond); /// Warunek do ostatnio dodanego przejścia (transition).
        template <typename Func> StateMachine *whenf (Func func) { return when (new FuncCondition<EventType, Func> (func)); }

        StateMachine *then (ActionType *action); /// Akcja do ostatnio dodanego przejścia (transition).
        template <typename Func> StateMachine *thenf (Func func) { return then (new FuncAction<EventType, Func> (func)); }

        StateMachine *defer (uint8_t stateLabel, bool global = false);

        EventQueue &getEventQueue () { return eventQueue; }
        EventQueue const &getEventQueue () const { return eventQueue; }

        void setInitialState (StateType *s);
        void setInitialState (uint8_t stateLabel);
        //        void addState (StateType *s);
        void addGlobalTransition (TransitionType *t, TransitionPriority run = TransitionPriority::RUN_LAST);
        void addGlobalRule (RuleType *r);
        void addDeferredEventCondition (Condition<EventT> *cond);

private:
        bool check (ConditionType &condition, uint8_t inputNum, EventType &retainedInput);
        bool skipEvent (EventT const &e) const;

private:
        StateType *lastAddedState = nullptr;
        TransitionType *lastAddedTransition = nullptr;
        TransitionType *lastAddedTransitionRF = nullptr;
        TransitionType *lastAddedTransitionRL = nullptr;
        ActionType *lastAddedAction = nullptr;
        RuleType *lastAddedRule = nullptr;

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

        StateType *initialState = nullptr;
        StateType *currentState = nullptr;
        TransitionType *firstTransitionRL = nullptr; /// Run last
        TransitionType *firstTransitionRF = nullptr; /// Run First
        RuleType *globalRule = nullptr;
        EventQueue eventQueue;
        DeferredEventContainer deferredEventQueue;
        StateType states[MAX_STATES_NUM];
        // Z kolejki jest kopiowane tutaj kiedy warunek zostanie spełniony i ma ustawione RETAIN
        EventType inputCopy;
        ActionQueue actionQueue;
        TimeCounter *timeCounter = nullptr;
        uint32_t logId;
        uint8_t *synchroCounter = nullptr;
        uint8_t synchroModulo = 0;

        /**
         * Kiedy true, to maszyna pracuje na jednym wejściu na raz. To oznacza, że warunki są sprawdzane tylko na jednym wejściu,
         * i potem jak przejscie zoatsnie znależione, to tylko ot jednojest zdejmowane. Jeśli to pole jest false, to pracuje na
         * całej kolejce (może być w niej wiele wejść). Jeśli znajdzie przejscie, to czyści całą kolejkę wejściową po znalezieniu
         * przejścia.
         * Domyślnie false
         */
        bool useOnlyOneInputAtATime;
        Condition<EventT> *deferredEventCondition = nullptr;
        Log log;
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

                actionQueue.pop ();
        }

        return true;
}

/*****************************************************************************/

template <typename EventT> bool StateMachine<EventT>::fixCurrentState ()
{
        if (!currentState) {
                currentState = initialState;

#if !defined(UNIT_TEST)
                if (log == Log::YES) {
                        uint8_t currentLabel = currentState->getLabel ();
                        debug->print ("fixCurrentState : ");
                        debug->println (currentLabel);
                }
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
        if (eventQueue.size ()) {
                for (int i = 0; i < inputNum; ++i) {
                        auto &e = eventQueue.at (i);

                        if (skipEvent (e)) {
                                continue;
                        }

                        if (condition.check (e, retainedInput)) {
                                break;
                        }
                }

                if (condition.getResult ()) {
                        return true;
                }
        }
        else {
                /*
                 * This was added for enabling checks on external variables, that is
                 * checks independent of the event queue. But I have doubts now. This
                 * implementation makes conditions like : like ("%") pointless, because
                 * it will catch this dummy event even though there was no "real" event
                 * i.e. there was no response from the modem.
                 */
                EventType tmp;
                condition.check (tmp, retainedInput);

                if (condition.getResult ()) {
                        return true;
                }
        }

        //        for (auto i = deferredEventQueue.begin (); i != deferredEventQueue.end (); ++i) {
        //                if (condition.check (*i, retainedInput)) {
        //                        deferredEventQueue.erase (i);
        //                        return true;
        //                }
        //        }

        return false;
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

        uint8_t noOfInputs = 0;
        {
                InterruptLock<CortexMInterruptControl> lock;
                noOfInputs = (useOnlyOneInputAtATime) ? (1) : (eventQueue.size ());
        }

#if !defined(UNIT_TEST)
        if (log == Log::YES) {
                for (int i = 0; i < noOfInputs; ++i) {
                        EventT &ev = eventQueue.at (i);

                        if (skipEvent (ev)) {
                                continue;
                        }

                        // TODO hacked.
                        for (RuleType *rule = this->globalRule; rule != nullptr; rule = static_cast<RuleType *> (rule->next)) {
                                ConditionType *cond = rule->getCondition ();

                                if (cond->check (ev, inputCopy)) {
                                        ActionType *a = rule->getAction ();
                                        a->run (ev);
                                }
                        }

                        size_t size = ev.size ();
                        EventT copy;

                        auto endIter = ev.cbegin ();
                        std::advance (endIter, std::min<size_t> (size, MAX_LOG_LINE_SIZE));
                        std::copy_if (ev.cbegin (), endIter, std::back_inserter (copy),
                                      [] (auto const &chr) -> bool { return chr != '\r' && chr != '\n'; });

                        debug->print ("IN (");
                        debug->print (int (size));
                        debug->print (") : ");
                        debug->print ((uint8_t *)copy.data (), copy.size ());
                        debug->println ((size > MAX_LOG_LINE_SIZE) ? ("...") : (""));
                }
        }
#endif

        /*
         * TODO suboptimal : 1. Checks the condiution 2 times - here, and then when looking for a transition,
         * 2. copies events which may be heavy.
         */
        //        for (ConditionType *c = currentState->deferredEventCondition; c != nullptr; c = c->next) {
        //                for (int i = 0; i < noOfInputs; ++i) {
        //                        EventType &e = eventQueue.at (i);

        //                        if (c->check (e, inputCopy)) {
        //                                if (deferredEventQueue.full ()) {
        //                                        // TODO lepsza obsługa błędów.
        //                                        Error_Handler ();
        //                                }

        //                                deferredEventQueue.push_back (e);
        //                        }
        //                }
        //        }

        //        // TODO this is copy pasted.
        //        for (ConditionType *c = deferredEventCondition; c != nullptr; c = c->next) {
        //                for (int i = 0; i < noOfInputs; ++i) {
        //                        EventType &e = eventQueue.at (i);

        //                        if (c->check (e, inputCopy)) {
        //                                if (deferredEventQueue.full ()) {
        //                                        // TODO lepsza obsługa błędów.
        //                                        Error_Handler ();
        //                                }

        //                                deferredEventQueue.push_back (e);
        //                        }
        //                }
        //        }

        while (true) {
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

        {
                InterruptLock<CortexMInterruptControl> lock;
                for (int i = 0; i < noOfInputs; ++i) {
                        eventQueue.pop_front ();
                }
        }

        /*
         * Conditions are stateful i.e. they remember the result of last check,
         * so here we reset their state.
         */
        if (ret && ret->getCondition ()) {
                ret->getCondition ()->reset ();
        }

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

        currentState = &states[t->getTo ()];

        if (!currentState) {
                errorCondition (NO_SUCH_STATE);
        }

#if !defined(UNIT_TEST)
        if (log == Log::YES) {
                uint8_t currentLabel = currentState->getLabel ();
                debug->print ("transition : ");
                debug->println (currentLabel);
        }
#endif

        if (timeCounter) {
                timeCounter->set ();
        }

        pushBackAction (currentState->getEntryAction ());
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

        // TODO hack - jeżeli to jest 0, to nie robimy przejścia, jedynie uruchamiamy akcję transition
        // TODO zastąpiłem ten mechanizm za pomocą Rule
        if (t->getTo () == 0) {
                pushBackAction (t->getAction ());
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

template <typename EventT> void StateMachine<EventT>::setInitialState (uint8_t stateLabel) { initialState = &states[stateLabel]; }

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

template <typename EventT> void StateMachine<EventT>::addGlobalRule (RuleType *r)
{
        if (!lastAddedRule) {
                globalRule = lastAddedRule = r;
        }
        else {
                lastAddedRule->next = r;
                lastAddedRule = r;
        }
}

/*****************************************************************************/

// template <typename EventT> void StateMachine<EventT>::addState (StateType *s)
//{
//        if (s->getLabel () >= MAX_STATES_NUM) {
//                errorCondition (STATES_ARRAY_FULL);
//        }

//        states[s->getLabel ()] = s;
//}

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::pushBackAction (ActionType *a)
{
        if (!a) {
                return;
        }

        if (actionQueue.full ()) {
                errorCondition (OPERATION_QUEUE_FULL);
        }

        actionQueue.push (a);
}

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::errorCondition (Error)
{
#ifdef UNIT_TEST
        abort ();
#endif

        while (true) {
        }
}

/*****************************************************************************/

template <typename EventT> StateMachine<EventT> *StateMachine<EventT>::state (uint8_t label, StateFlags flags)
{
        lastAddedState = &states[label];
        lastAddedState->label = label;
        lastAddedState->setFlags (flags);
        //        addState (lastAddedState);

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

template <typename EventT> StateMachine<EventT> *StateMachine<EventT>::rule ()
{
        addGlobalRule (new RuleType (nullptr, 0));
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
        {
                InterruptLock<CortexMInterruptControl> lock;
                eventQueue.clear ();
        }

        currentState = nullptr;

        // Przez to TimeCountery wariowały. Ticket #83
        // if (timeCounter) {
        //         timeCounter->reset ();
        // }

        inputCopy.clear ();
        actionQueue.clear ();
}

/*****************************************************************************/

template <typename EventT> uint8_t StateMachine<EventT>::getCurrentStateLabel () const
{
        if (currentState) {
                return currentState->getLabel ();
        }

        return 0;
}

/*****************************************************************************/

template <typename EventT> StateMachine<EventT> *StateMachine<EventT>::defer (uint8_t stateLabel, bool global)
{
        if (!lastAddedTransition && !lastAddedTransition->getCondition ()) {
                errorCondition (NO_LAST_ADDED_TRANSITION);
        }

        if (global) {
                addDeferredEventCondition (lastAddedTransition->getCondition ());
        }
        else {
                StateType &state = states[stateLabel];
                state.addDeferredEventCondition (lastAddedTransition->getCondition ());
        }
        return this;
}

/*****************************************************************************/

template <typename EventT> void StateMachine<EventT>::addDeferredEventCondition (Condition<EventT> *cond)
{
        if (!deferredEventCondition) {
                deferredEventCondition = cond;
                return;
        }

        for (auto c = deferredEventCondition; c != nullptr; c = c->next) {
                if (!c->next) {
                        c->next = cond;
                        return;
                }
        }
}

/*****************************************************************************/

template <typename E> bool StateMachine<E>::skipEvent (E const &e) const
{
        auto size = e.size ();

        if (size == 0) {
                return true;
        }

        auto c1 = e.at (0);

        if (c1 == '\n' || c1 == '\r') {
                if (size == 1) {
                        return true;
                }

                auto c2 = e.at (1);
                if (c2 == '\n' || c2 == '\r') {
                        if (size == 2) {
                                return true;
                        }
                }
        }

        return false;
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
