/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef ANDCONDITION_H
#define ANDCONDITION_H

#include "Condition.h"

/**
 * @brief Jak nazwa wskazuje.
 */
template <typename EventT = string> class AndCondition : public Condition<EventT> {
public:
        using EventType = EventT;

        AndCondition (Condition<EventType> *a, Condition<EventType> *b) : a (a), b (b) {}
        virtual ~AndCondition () override = default;

        bool getResult () const override { return a->getResult () && b->getResult (); }
        void reset () override
        {
                a->reset ();
                b->reset ();
        }

        virtual bool check (EventType const &event, EventType &retainedEvent) const override
        {
                if (!a->getResult ()) {
                        a->check (event, retainedEvent);
                }

                if (!b->getResult ()) {
                        b->check (event, retainedEvent);
                }

                return getResult ();
        }

private:
        Condition<EventType> *a;
        Condition<EventType> *b;
};

/*****************************************************************************/

template <typename EventT = string> AndCondition<EventT> *anded (Condition<EventT> *a, Condition<EventT> *b)
{
        return new AndCondition<EventT> (a, b);
}

/**
 * Sequence is like AndCondition, but inputs must fulfill conditions in order.
 */
template <typename EventT = string> class SequenceCondition : public Condition<EventT> {
public:
        using EventType = EventT;

        SequenceCondition (Condition<EventT> &a, Condition<EventT> &b) : a (a), b (b) {}
        virtual ~SequenceCondition () = default;

        bool getResult () const override { return a.getResult () && b.getResult (); }
        void reset () override
        {
                a.reset ();
                b.reset ();
        }

        virtual bool check (EventType const &event, EventType &retainedEvent) const override
        {
                if (!a.getResult ()) {
                        a.check (event, retainedEvent);
                }

                if (a.getResult () && !b.getResult ()) {
                        b.check (event, retainedEvent);
                }

                return getResult ();
        }

private:
        Condition<EventType> &a;
        Condition<EventType> &b;
};

template <typename EventT = string> SequenceCondition<EventT> *seq (Condition<EventT> &a, Condition<EventT> &b)
{
        return new SequenceCondition<EventT> (a, b);
}

#endif // ANDCONDITION_H
