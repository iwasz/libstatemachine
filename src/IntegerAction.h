/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef INTEGERACTION_H
#define INTEGERACTION_H

#include "Action.h"
#include "Timer.h"

enum IntegerActionType { INC, DEC, CLEAR, SET_ACTUAL_TIME };

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class IntegerAction : public Action<EventT> {
public:
        using EventType = EventT;

        IntegerAction (int *i, IntegerActionType a) : integer (i), action (a) {}
        virtual ~IntegerAction () = default;

        virtual bool run (EventType const &event);

private:
        int *integer;
        IntegerActionType action;
};

/*****************************************************************************/

template <typename EventT> bool IntegerAction<EventT>::run (EventType const &event)
{
        switch (action) {
        case INC:
                ++(*integer);
                break;

        case DEC:
                --(*integer);
                break;

        case CLEAR:
                *integer = 0;
                break;

        case SET_ACTUAL_TIME:
                *integer = Timer::getTick ();
                break;
        };

        return true;
}

#endif // INTEGERACTION_H
