/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "AndCondition.h"
#include <cstdio>
#include <cstring>

#if 0
bool AndCondition::check (EventQueue &eventQueue, uint8_t inputNum, EventType &retainedInput) const
{
        bool conditionAMet = false;
        bool conditionBMet = false;

        //        bool i = false;
        EventType *el;

        if (eventQueue.size ()) {
                uint8_t i = 0;
                while (i < inputNum && (el = &eventQueue.front (i++)) && (!conditionAMet || !conditionBMet)) {

                        if (a && !conditionAMet) {
                                conditionAMet = a->check (*el, retainedInput);
#if 0 & !defined(UNIT_TEST)
                                if (conditionAMet) {
                                        printf ("conditionAMet : [%s]\n", el->data);
                                }
#endif
                        }

                        if (b && !conditionBMet) {
                                conditionBMet = b->check (*el, retainedInput);
#if 0 & !defined(UNIT_TEST)
                                if (conditionBMet) {
                                        printf ("condition[B]Met : [%s]\n", el->data);
                                }
#endif
                        }
                }
        }
        else {
                if (a && !conditionAMet) {
                        conditionAMet = a->check (EventType (), retainedInput);
                }

                if (b && !conditionBMet) {
                        conditionBMet = b->check (EventType (), retainedInput);
                }
        }

        if (conditionAMet && conditionBMet) {
                conditionAMet = conditionBMet = false;
                return true;
        }

        return false;
}
#endif

/*****************************************************************************/

AndCondition *anded (Condition *a, Condition *b /*, ICondition *c, ICondition *d, ICondition *e*/) { return new AndCondition (a, b); }

/*****************************************************************************/

#if 0
bool SequenceCondition::check (EventQueue &eventQueue, uint8_t inputNum, EventType &retainedInput) const
{
        bool conditionAMet = false;
        bool conditionBMet = false;

        EventType *el;

        if (eventQueue.size ()) {
                uint8_t i = 0;
                while (i < inputNum && (el = &eventQueue.front (i++)) && (!conditionAMet || !conditionBMet)) {

                        if (!conditionAMet) {
                                conditionAMet = a.check (*el, retainedInput);
#if 0 & !defined(UNIT_TEST)
                                if (conditionAMet) {
                                        printf ("conditionAMet : [%s]\n", el->data);
                                }
#endif
                        }

                        if (conditionAMet && !conditionBMet) {
                                conditionBMet = b.check (*el, retainedInput);
#if 0 & !defined(UNIT_TEST)
                                if (conditionBMet) {
                                        printf ("condition[B]Met : [%s]\n", el->data);
                                }
#endif
                        }
                }
        }
        else {
                if (!conditionAMet) {
                        conditionAMet = a.check (EventType (), retainedInput);
                }

                if (conditionAMet && !conditionBMet) {
                        conditionBMet = b.check (EventType (), retainedInput);
                }
        }

        if (conditionAMet && conditionBMet) {
                conditionAMet = conditionBMet = false;
                return true;
        }

        return false;
}
#endif

/*****************************************************************************/

SequenceCondition *seq (Condition &a, Condition &b) { return new SequenceCondition (a, b); }
