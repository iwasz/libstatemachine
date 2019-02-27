/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "StateMachine.h"
#include <cstring>
#ifndef UNIT_TEST
#include "Debug.h"
#endif

/*****************************************************************************/

bool StateMachine::fireActions ()
{
        while (actionQueue.size ()) {
                Action *op = actionQueue.front ();
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

bool StateMachine::fixCurrentState ()
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

Transition *StateMachine::findTransition (uint8_t noOfInputs)
{
        Transition *ret = nullptr;
        enum Stage { FIRSTS, REGULARS, LASTS };
        Stage stage;
        Transition *t;
        bool suppressGlobalTransitions = currentState->getFlags () & State::SUPPRESS_GLOBAL_TRANSITIONS;

        if (suppressGlobalTransitions) {
                stage = REGULARS;
                t = currentState->getFirstTransition ();
        }
        else {
                stage = FIRSTS;
                t = firstTransitionRF;
        }

        while (1) {

                if (!t || !t->getCondition () || !t->getCondition ()->check (eventQueue, noOfInputs, inputCopy)) {

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

        return ret;
}

/*****************************************************************************/

void StateMachine::performTransition (Transition *t)
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

void StateMachine::run ()
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

#ifndef UNIT_TEST
        __disable_irq ();
#endif

        uint8_t noOfInputs = (useOnlyOneInputAtATime) ? (1) : (eventQueue.size ());

#ifndef UNIT_TEST
        __enable_irq ();
#endif

        Transition *t = findTransition (noOfInputs);

        if (!t) {
                return;
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

        performTransition (t);

        if (currentState && (currentState->getFlags () & State::INC_SYNCHRO)) {
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

void StateMachine::setInitialState (State *s) { initialState = s; }

/*****************************************************************************/

void StateMachine::setInitialState (uint8_t stateLabel) { initialState = states[stateLabel]; }

/*****************************************************************************/

void StateMachine::addGlobalTransition (Transition *t, Transition::Type run)
{
        if (run == Transition::RUN_LAST) {
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

void StateMachine::addState (State *s)
{
        if (s->getLabel () >= MAX_STATES_NUM) {
                errorCondition (STATES_ARRAY_FULL);
        }

        states[s->getLabel ()] = s;
}

/*****************************************************************************/

void StateMachine::pushBackAction (Action *a)
{
        if (!a) {
                return;
        }

        //        do {
        //                pushBackOperation (a);
        if (!actionQueue.push_back ()) {
                errorCondition (OPERATION_QUEUE_FULL);
        }

        ActionQueue::Element &el = actionQueue.back ();
        el = a;
        //        } while ((a = a->getNext ()));
}

/*****************************************************************************/

void StateMachine::errorCondition (Error e)
{
        while (true) {
        }
}

/*****************************************************************************/

StateMachine *StateMachine::state (uint8_t label, uint8_t flags)
{
        lastAddedState = new State (label);
        lastAddedState->setFlags (flags);
        addState (lastAddedState);

        if (flags & State::INITIAL) {
                setInitialState (lastAddedState);
        }

        return this;
}

/*****************************************************************************/

StateMachine *StateMachine::entry (Action *action)
{
        if (!lastAddedState) {
                errorCondition (NO_LAST_ADDED_STATE);
        }

        lastAddedAction = action;
        lastAddedState->setEntryAction (action);
        return this;
}

/*****************************************************************************/

StateMachine *StateMachine::exit (Action *action)
{
        if (!lastAddedState) {
                errorCondition (NO_LAST_ADDED_STATE);
        }

        lastAddedAction = action;
        lastAddedState->setExitAction (action);
        return this;
}

/*****************************************************************************/

// StateMachine *StateMachine::and_action (Action *action)
//{
//        if (!lastAddedAction) {
//                errorCondition (NO_LAST_ADDED_ACTION);
//        }

//        lastAddedAction->setNext (action);
//        lastAddedAction = action;
//        return this;
//}

/*****************************************************************************/

StateMachine *StateMachine::transition (uint8_t to, Transition::Type run)
{
        if (!lastAddedState) {
                // errorCondition (NO_LAST_ADDED_STATE);
                addGlobalTransition (new Transition (nullptr, to), run);
        }
        else {
                lastAddedTransition = new Transition (nullptr, to);
                lastAddedState->addTransition (lastAddedTransition);
        }
        return this;
}

/*****************************************************************************/

StateMachine *StateMachine::when (Condition *cond)
{
        if (!lastAddedTransition) {
                errorCondition (NO_LAST_ADDED_TRANSITION);
        }

        lastAddedTransition->setCondition (cond);
        return this;
}

/*****************************************************************************/

StateMachine *StateMachine::then (Action *action)
{
        if (!lastAddedTransition) {
                errorCondition (NO_LAST_ADDED_TRANSITION);
        }

        lastAddedTransition->setAction (action);
        lastAddedAction = action;
        return this;
}

/*****************************************************************************/

void StateMachine::reset (/*uint8_t state*/)
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

uint8_t StateMachine::getCurrentStateLabel () const
{
        if (currentState) {
                return currentState->getLabel ();
        }
        else {
                return 0;
        }
}
