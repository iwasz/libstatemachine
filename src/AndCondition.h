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
class AndCondition : public Condition {
public:
        AndCondition (Condition *a, Condition *b) : a (a) /*, conditionAMet (false)*/, b (b) /*, conditionBMet (false)*/ {}
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
        Condition *a;
        Condition *b;
};

extern AndCondition *anded (Condition *a, Condition *b /*, ICondition *c = nullptr, ICondition *d = nullptr, ICondition *e = nullptr*/);

/**
 * Sequence is like AndCondition, but inputs must fulfill conditions in order.
 */
class SequenceCondition : public Condition {
public:
        SequenceCondition (Condition &a, Condition &b) : a (a), b (b) {}
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
        Condition &a;
        Condition &b;
};

extern SequenceCondition *seq (Condition const &a, Condition const &b);
#endif // ANDCONDITION_H
