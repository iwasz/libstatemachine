/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef STATE_MACHINE_STATE_H
#define STATE_MACHINE_STATE_H

#include "StateMachineTypes.h"
#include <cstdint>
#include <type_traits>

template <typename EventT> class Transition;
template <typename EventT> class Action;
template <typename EventT> class Condition;

/// bit fields
enum class StateFlags : uint8_t { NONE = 0x00, INITIAL = 0x01, INC_SYNCHRO = 0x02, SUPPRESS_GLOBAL_TRANSITIONS = 0x04 };

/*****************************************************************************/

inline StateFlags operator| (StateFlags lhs, StateFlags rhs)
{
        using T = std::underlying_type_t<StateFlags>;
        return static_cast<StateFlags> (static_cast<T> (lhs) | static_cast<T> (rhs));
}

/*****************************************************************************/

inline StateFlags operator& (StateFlags lhs, StateFlags rhs)
{
        using T = std::underlying_type_t<StateFlags>;
        return static_cast<StateFlags> (static_cast<T> (lhs) & static_cast<T> (rhs));
}

/**
 * @brief Stan. Uwaga, label nie może być 0.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class State {
public:
        State () = default;

        Action<EventT> *getEntryAction () { return entryAction; }
        void setEntryAction (Action<EventT> *a) { entryAction = a; }

        Action<EventT> *getExitAction () { return exitAction; }
        void setExitAction (Action<EventT> *a) { exitAction = a; }

        uint8_t getLabel () const { return label; }

        void addTransition (Transition<EventT> *t);
        Transition<EventT> *getFirstTransition () { return firstTransition; }

        StateFlags getFlags () const { return flags; }
        void setFlags (StateFlags value) { flags = value; }

        void addDeferredEventCondition (Condition<EventT> *cond);

#ifndef UNIT_TEST
private:
#endif
        template <typename T> friend class StateMachine;

        Action<EventT> *entryAction = nullptr;
        Action<EventT> *exitAction = nullptr;
        uint8_t label = 0;
        Transition<EventT> *firstTransition = nullptr;
        Transition<EventT> *lastAddedTransition = nullptr;
        StateFlags flags = StateFlags::NONE;
        Condition<EventT> *deferredEventCondition = nullptr;
};

/*****************************************************************************/

template <typename EventT> void State<EventT>::addTransition (Transition<EventT> *t)
{
        if (!lastAddedTransition) {
                firstTransition = lastAddedTransition = t;
        }
        else {
                lastAddedTransition->next = t;
                lastAddedTransition = t;
        }
}

/*****************************************************************************/

template <typename EventT> void State<EventT>::addDeferredEventCondition (Condition<EventT> *cond)
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

#endif // STATE_H
