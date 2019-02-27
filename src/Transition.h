#ifndef STATE_MACHINE_TRANSITION_H
#define STATE_MACHINE_TRANSITION_H

#include "Action.h"
#include "State.h"
#include "Condition.h"

class Transition {
public:
        enum Type { RUN_FIRST, RUN_LAST };

        Transition (Condition *c = nullptr, /*uint8_t from = 0,*/ uint8_t to = 0, Action *action = nullptr)
            : condition (c), /*from (from),*/ to (to), action (action), next (nullptr)
        {
        }

        void setCondition (Condition *value) { condition = value; }
        Condition *getCondition () { return condition; }

//        uint8_t getFrom () { return from; }
//        void setFrom (uint8_t value) { from = value; }

        uint8_t getTo () { return to; }
        void setTo (uint8_t value) { to = value; }

        Action *getAction () { return action; }
        void setAction (Action *value) { action = value; }

private:
        Condition *condition;
//        uint8_t from;
        uint8_t to;
        Action *action;

        friend class StateMachine;
        friend class State;
        Transition *next;
};

#endif // TRANSITION_H
