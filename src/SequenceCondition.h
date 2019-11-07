/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include "Condition.h"
#include <gsl/gsl>

/**
 * Sequence is like AndCondition, but inputs must fulfill conditions in order.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class SequenceCondition : public Condition<EventT> {
public:
        using EventType = EventT;

        SequenceCondition (Condition<EventT> *a, Condition<EventT> *b) : a (a), b (b) {}
        virtual ~SequenceCondition () = default;

        bool getResult () const override { return a->getResult () && b->getResult (); }
        void reset () override
        {
                a->reset ();
                b->reset ();
        }

        bool check (EventType  &event, EventType &retainedEvent) const override
        {
                if (!a->getResult ()) {
                        a->check (event, retainedEvent);
                }

                if (a->getResult () && !b->getResult ()) {
                        b->check (event, retainedEvent);
                }

                return getResult ();
        }

private:
        gsl::not_null <Condition<EventType>*> a;
        gsl::not_null <Condition<EventType>*> b;
};

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> SequenceCondition<EventT> *seq (Condition<EventT> *a, Condition<EventT> *b)
{
        return new SequenceCondition<EventT> (a, b);
}

