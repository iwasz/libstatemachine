#ifndef STATE_MACHINE_STATE_H
#define STATE_MACHINE_STATE_H

#include "StateMachineTypes.h"
#include <cstdint>
#include <type_traits>

template <typename EventT> class Transition;
template <typename EventT> class Action;

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
template <typename EventT = string> class State {
public:
        /**
         * @brief State
         * @param label Służy do odróżniania stanów (nie są rozróżniane po wskaźnikach). Nie może być 0, ale nie ma sprawdzenia!
         * @param entryAction
         * @param exitAction
         */
        State (uint8_t label, Action<EventT> *entryAction = nullptr, Action<EventT> *exitAction = nullptr)
            : entryAction (entryAction),
              exitAction (exitAction),
              label (label),
              firstTransition (nullptr),
              lastAddedTransition (nullptr),
              flags (StateFlags::INITIAL | StateFlags::INC_SYNCHRO)
        {
        }

        Action<EventT> *getEntryAction () { return entryAction; }
        void setEntryAction (Action<EventT> *a) { entryAction = a; }

        Action<EventT> *getExitAction () { return exitAction; }
        void setExitAction (Action<EventT> *a) { exitAction = a; }

        uint8_t getLabel () const { return label; }

        void addTransition (Transition<EventT> *t);
        Transition<EventT> *getFirstTransition () { return firstTransition; }

        StateFlags getFlags () const { return flags; }
        void setFlags (StateFlags value) { flags = value; }

#ifndef UNIT_TEST
private:
#endif

        Action<EventT> *entryAction;
        Action<EventT> *exitAction;
        uint8_t label;
        Transition<EventT> *firstTransition;
        Transition<EventT> *lastAddedTransition;
        StateFlags flags;
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

#endif // STATE_H
