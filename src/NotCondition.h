/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef NOTCONDITION_H
#define NOTCONDITION_H

#include "Condition.h"

/**
 *
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class NotCondition : public Condition<EventT> {
public:
        using EventType = EventT;

        NotCondition (Condition<EventT> *c) : cond (c) {}
        virtual ~NotCondition () {}

        bool getResult () const override { return !cond->getResult (); }
        void reset () override { cond->reset (); }
        virtual bool check (EventType const &event, EventType &retainedEvent) const override { return !cond->check (event, retainedEvent); }

protected:
        bool checkImpl (const char *data) const { return !cond->checkImpl (data); }
        Condition<EventT> *cond;
};

/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> NotCondition<EventT> *negated (Condition<EventT> *c)
{
        return new NotCondition<EventT> (c);
}

#endif // NOTCONDITION_H
