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
class OrCondition : public Condition {
public:
        OrCondition (Condition *a, Condition *b) : a (a) /*, conditionAMet (false)*/, b (b) /*, conditionBMet (false)*/ {}

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
        Condition *a;
        Condition *b;
};

extern OrCondition *ored (Condition *a, Condition *b /*, Condition *c = nullptr, Condition *d = nullptr, Condition *e = nullptr*/);

#endif // ORCONDITION_H
