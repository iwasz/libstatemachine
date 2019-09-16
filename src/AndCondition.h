/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include "Condition.h"

/**
 * Checks if both conditions are met, doesn't care about event order. So if we
 * have say (meta code) : anded (eq ("OK"), eq ("ATZ")), then there is no difference
 * if "OK" or "ATZ" event shows up first.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class AndCondition : public Condition<EventT> {
public:
        using EventType = EventT;

        AndCondition (Condition<EventType> *a, Condition<EventType> *b) : a (a), b (b) {}
        ~AndCondition () override = default;

        bool getResult () const override { return a->getResult () && b->getResult (); }
        void reset () override
        {
                a->reset ();
                b->reset ();
        }

        bool check (EventType const &event) const override
        {
                if (!a->getResult ()) {
                        a->check (event);
                }

                if (!b->getResult ()) {
                        b->check (event);
                }

                return getResult ();
        }

private:
        Condition<EventType> *a;
        Condition<EventType> *b;
};

/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> AndCondition<EventT> *anded (Condition<EventT> *a, Condition<EventT> *b)
{
        return new AndCondition<EventT> (a, b);
}
