/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef STATECONDITION_H
#define STATECONDITION_H

#include "Condition.h"
#include <cstdint>

template <typename T> class StateMachine;

/**
 * @brief The StateCondition class
 */
template <typename StateMachineT, typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class StateCondition : public Condition<EventT> {
public:
        using EventType = EventT;
        using StateMachineType = StateMachineT;

        StateCondition (StateMachineType *g, uint8_t sl) : machine (g), stateLabel (sl) {}
        virtual ~StateCondition () {}

#ifndef UNIT_TEST
private:
#endif

        bool checkImpl (EventType const &) const { return machine->getCurrentStateLabel () == stateLabel; }

        StateMachineType *machine;
        uint8_t stateLabel;
};

#endif // STATECONDITION_H
