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
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class BoolCondition : public Condition<EventT> {
public:
        using EventType = EventT;
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
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class TrueCondition : public Condition<EventT> {
public:
        using EventType = EventT;
        TrueCondition (InputRetention r = InputRetention::IGNORE_INPUT) : Condition<EventType> (r) {}
        virtual ~TrueCondition () override = default;
        bool checkImpl (EventType const &) const override { return true; }
};

static TrueCondition<> alwaysTrue;

/**
 * Always false condition.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class FalseCondition : public Condition<EventT> {
public:
        using EventType = EventT;
        // Cannot retain, so no constructor.
        virtual ~FalseCondition () override = default;
        bool checkImpl (EventType const &) const override { return false; }
};

#endif // CONSTCONDITION_H
