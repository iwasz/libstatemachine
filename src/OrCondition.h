/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef ORCONDITION_H
#define ORCONDITION_H

#include "Condition.h"

/**
 * @brief Jak nazwa wskazuje. Do 5 operandów.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class OrCondition : public Condition<EventT> {
public:
        using EventType = EventT;

        OrCondition (Condition<EventT> *a, Condition<EventT> *b) : a (a) /*, conditionAMet (false)*/, b (b) /*, conditionBMet (false)*/ {}

        bool getResult () const override { return a->getResult () || b->getResult (); }
        void reset () override
        {
                a->reset ();
                b->reset ();
        }

        virtual bool check (EventType const &event, EventType &retainedEvent) const override
        {
                if (!a->getResult () && !b->getResult ()) {
                        a->check (event, retainedEvent);
                }

                if (!a->getResult () && !b->getResult ()) {
                        b->check (event, retainedEvent);
                }

                return getResult ();
        }

private:
        Condition<EventT> *a;
        Condition<EventT> *b;
};

/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> OrCondition<EventT> *ored (Condition<EventT> *a, Condition<EventT> *b)
{
        return new OrCondition<EventT> (a, b);
}

#endif // ORCONDITION_H
