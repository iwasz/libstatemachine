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
        virtual ~AndCondition () {}

        bool check (EventQueue &eventQueue, uint8_t inputNum, EventType &retainedInput) const;

private:
        bool checkImpl (EventType const &) const
        {
                Error_Handler ();
                return false;
        }

        Condition *a;
        Condition *b;
};

extern AndCondition *anded (Condition *a, Condition *b /*, ICondition *c = nullptr, ICondition *d = nullptr, ICondition *e = nullptr*/);

/**
 * Sequence is like AndCondition, but inputs must fulfill conditions in order.
 */
class SequenceCondition : public Condition {
public:
        SequenceCondition (Condition const &a, Condition const &b) : a (a), b (b) {}
        virtual ~SequenceCondition () = default;

        bool check (EventQueue &eventQueue, uint8_t inputNum, EventType &retainedInput) const;

private:
        bool checkImpl (EventType const &) const
        {
                Error_Handler ();
                return false;
        }

        Condition const &a;
        Condition const &b;
};

extern SequenceCondition *seq (Condition const &a, Condition const &b);
#endif // ANDCONDITION_H
