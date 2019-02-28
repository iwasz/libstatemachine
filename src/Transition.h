#ifndef STATE_MACHINE_TRANSITION_H
#define STATE_MACHINE_TRANSITION_H

#include "Action.h"
#include "Condition.h"
#include "State.h"

enum class TransitionPriority { RUN_FIRST, RUN_LAST };

template <typename EventT = string> class Transition {
public:
        using EventType = EventT;

        Transition (Condition<EventType> *c = nullptr, uint8_t to = 0, Action<EventType> *action = nullptr)
            : condition (c), to (to), action (action), next (nullptr)
        {
        }

        void setCondition (Condition<EventType> *value) { condition = value; }
        Condition<EventType> *getCondition () { return condition; }

        uint8_t getTo () { return to; }
        void setTo (uint8_t value) { to = value; }

        Action<EventType> *getAction () { return action; }
        void setAction (Action<EventType> *value) { action = value; }

private:
        template <typename T> friend class StateMachine;
        template <typename T> friend class State;

        Condition<EventType> *condition;
        uint8_t to;
        Action<EventType> *action;
        Transition *next;
};

#endif // TRANSITION_H
