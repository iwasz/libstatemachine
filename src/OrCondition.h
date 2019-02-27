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
        OrCondition (uint8_t num);
        virtual ~OrCondition ()
        { /* delete [] conditions */
        }
        void addCondition (uint8_t i, Condition *c) { conditions[i] = c; }

private:
        bool checkAndRetain (EventType const &event, EventType &retainedEvent) const;
        bool checkImpl (EventType const &event) const;

private:
        Condition **conditions;
        uint8_t numConditions;
};

extern OrCondition *ored (Condition *a, Condition *b, Condition *c = nullptr, Condition *d = nullptr, Condition *e = nullptr);

#endif // ORCONDITION_H
