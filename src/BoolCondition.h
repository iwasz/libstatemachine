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
        virtual ~BoolCondition () = default;

private:
        bool checkImpl (EventType const &) const { return *b; }
        bool *b;
};

/**
 * Always true condition.
 */
class TrueCondition : public Condition {
public:
        TrueCondition (InputRetention r = IGNORE_INPUT) : Condition (r) {}
        virtual ~TrueCondition () = default;

private:
        bool checkImpl (EventType const &) const { return true; }
};

static TrueCondition alwaysTrue;

/**
 * Always false condition.
 */
class FalseCondition : public Condition {
public:
        virtual ~FalseCondition () = default;

private:
        bool checkImpl (EventType const &) const { return false; }
};

#endif // CONSTCONDITION_H
