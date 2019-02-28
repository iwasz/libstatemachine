/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "OrCondition.h"

//OrCondition::OrCondition (uint8_t num) : numConditions (num) { conditions = new Condition *[num]; }

//bool OrCondition::check (EventType const &event, EventType &retainedEvent) const
//{
//        bool ret = false;

//        for (int i = 0; !ret && i < numConditions; ++i) {
//                ret |= conditions[i]->check (event, retainedEvent);
//        }

//        return ret;
//}

/*****************************************************************************/

//bool OrCondition::checkImpl (const EventType &event) const
//{
//        bool ret = false;
//        for (int i = 0; !ret && i < numConditions; ++i) {
//                ret |= conditions[i]->checkImpl (event);
//        }
//        return ret;
//}

/*****************************************************************************/

OrCondition *ored (Condition *a, Condition *b/*, Condition *c, Condition *d, Condition *e*/)
{
//        OrCondition *orCond = new OrCondition (2 + bool(c) + bool(d) + bool(e));
//        orCond->addCondition (0, a);
//        orCond->addCondition (1, b);

//        if (c) {
//                orCond->addCondition (2, c);
//        }

//        if (d) {
//                orCond->addCondition (3, d);
//        }

//        if (e) {
//                orCond->addCondition (4, e);
//        }

//        return orCond;
    return new OrCondition (a, b);
}
