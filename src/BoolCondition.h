/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef BOOLEAN_CONDITION_H
#define BOOLEAN_CONDITION_H

#include "Condition.h"

/**
 * @brief Zwraca czy *B jest true.
 */
class BoolCondition : public Condition {
public:
        BoolCondition (bool *b) : b (b) {}
        virtual ~BoolCondition () override = default;

protected:
        bool checkImpl (EventType const &) const override { return *b; }

private:
        bool *b;
};

/**
 * Always true condition.
 */
class TrueCondition : public Condition {
public:
        TrueCondition (InputRetention r = IGNORE_INPUT) : Condition (r) {}
        virtual ~TrueCondition () override = default;
        bool checkImpl (EventType const &) const override { return true; }
};

static TrueCondition alwaysTrue;

/**
 * Always false condition.
 */
class FalseCondition : public Condition {
public:
        // Cannot retain, so no constructor.
        virtual ~FalseCondition () override = default;
        bool checkImpl (EventType const &) const override { return false; }
};

#endif // CONSTCONDITION_H
