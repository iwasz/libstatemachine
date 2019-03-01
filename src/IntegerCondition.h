/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef INTEGERCONDITION_H
#define INTEGERCONDITION_H

#include "Condition.h"
#include "Timer.h"

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class IntegerCondition : public Condition<EventT> {
public:
        using EventType = EventT;
        enum Type { GT, LT, EQ, TIME_PASSED_GT };
        virtual ~IntegerCondition () = default;

        IntegerCondition (int *i, Type t, int c) : integer (i), type (t), compareTo (c), compareToPtr (&compareTo) {}
        IntegerCondition (int *i, Type t, int *c) : integer (i), type (t), compareTo (0), compareToPtr (c) {}

protected:
        bool checkImpl (EventType const &event) const;

private:
        int *integer;
        Type type;
        int compareTo;
        int *compareToPtr;
};

/*****************************************************************************/

template <typename EventT> bool IntegerCondition<EventT>::checkImpl (EventType const &) const
{
#if 0
        // printf ("intAct i = [%d], *cTo = [%d], action = [%d]\n", *integer, *compareToPtr, (int)type);
        debug->print ("*integer : ");
        debug->print (*integer);
        debug->print (", *compareToPtr: ");
        debug->println (*compareToPtr);
#endif
        switch (type) {
        case GT:
                return *integer > *compareToPtr;
                break;

        case LT:
                return *integer < *compareToPtr;
                break;

        case EQ:
                return *integer == *compareToPtr;
                break;

        case TIME_PASSED_GT:
                return (Timer::getTick () - *integer) > static_cast<uint32_t> (*compareToPtr);
                break;

        default:
                break;
        }

        return false;
}

#endif // INTEGERCONDITION_H
