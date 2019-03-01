/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef SLEEP_STAGE_1_DACTION_H
#define SLEEP_STAGE_1_DACTION_H

#include "Action.h"
#include <cstdint>

template <typename T> class StateMachine;

/**
 * @brief
 */
template <typename StateMachineT, typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class StateChangeAction : public Action<EventT> {
public:
        using EventType = EventT;
        using StateMachineType = StateMachineT;

        StateChangeAction (StateMachineType *g, uint8_t sl) : machine (g), stateLabel (sl) {}
        virtual ~StateChangeAction () {}
        virtual bool run (EventType const &event);

protected:
        StateMachineType *machine;
        uint8_t stateLabel;
};

/*****************************************************************************/

template <typename StateMachineT, typename EventT> bool StateChangeAction<StateMachineT, EventT>::run (EventType const &)
{
        machine->setInitialState (stateLabel);
        machine->reset ();
        return true;
}

#endif // SleepStage1Action_H
