/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef BEGINSWITHCONDITION_H
#define BEGINSWITHCONDITION_H

#include "StringCondition.h"

/**
 * @brief Porównuje wejście z napisaem który mu ustawiamy. Jeśli początek wejścia jest taki sam, to zwraca true.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class BeginsWithCondition : public StringCondition<EventT> {
public:
        using EventType = EventT;

        BeginsWithCondition (const char *condition, StripInput stripInput = StripInput::STRIP,
                             InputRetention retainInput = InputRetention::IGNORE_INPUT)
            : StringCondition<EventType> (condition, stripInput, false, retainInput)
        {
        }

        virtual ~BeginsWithCondition () = default;

#ifndef UNIT_TEST
private:
#endif

        bool checkImpl (EventType const &event) const;
};

/*****************************************************************************/

template <typename EventT> bool BeginsWithCondition<EventT>::checkImpl (EventType const &event) const
{
        auto &condition = StringCondition<EventType>::condition;
        auto stripInput = StringCondition<EventType>::stripInput;

        if (event.size () == 0) {
                return condition.empty ();
        }

        size_t ei = 0;
        size_t ci = 0;

        // Stripuj początek.
        if (stripInput == StripInput::STRIP) {
                while (ei < event.size () && std::isspace (event.at (ei))) {
                        ++ei;
                }
        }

        while (ei < event.size () && ci < condition.size () && (event.at (ei) == condition.at (ci))) {
                ++ei;
                ++ci;
        }

        return (ci == condition.size ());
}
/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE>
BeginsWithCondition<EventT> *beginsWith (const char *condition, StripInput stripInput = StripInput::STRIP,
                                         InputRetention retainInput = InputRetention::IGNORE_INPUT)
{
        return new BeginsWithCondition<EventT> (condition, stripInput, retainInput);
}

#endif // BEGINSWITHCONDITION_H
