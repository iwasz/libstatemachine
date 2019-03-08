/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef LIKE_CONDITION_CONDITION_H
#define LIKE_CONDITION_CONDITION_H

#include "StringCondition.h"

/**
 * @brief Porównuje wejście z napisaem który mu ustawiamy. Jeśli początek wejścia jest taki sam, to zwraca true.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class LikeCondition : public StringCondition<EventT> {
public:
        using EventType = EventT;

        LikeCondition (const char *condition, StripInput stripInput = StripInput::STRIP,
                       InputRetention retainInput = InputRetention::IGNORE_INPUT)
            : StringCondition<EventType> (condition, stripInput, false, retainInput)
        {
        }
        virtual ~LikeCondition () {}

#ifndef UNIT_TEST
protected:
#endif

        virtual bool checkImpl (EventType const &event) const;
};

/*
 * TODO przyjżeć się tej implementacji jak będę wyspany.
 */
template <typename EventT> bool LikeCondition<EventT>::checkImpl (EventType const &event) const
{
        auto &condition = StringCondition<EventType>::condition;
        auto stripInput = StringCondition<EventType>::stripInput;

//        if (event.empty ()) {
//                return condition.empty ();
//        }

        size_t ei = 0;
        size_t ci = 0;

        // Stripuj początek.
        if (stripInput == StripInput::STRIP) {
                while (ei < event.size () && std::isspace (event.at (ei))) {
                        ++ei;
                }
        }

        bool resume;
        do {
                resume = false;

                // Normalny tekst, zgadzają się.
                while (ei < event.size () && ci < condition.size () && (event.at (ei) == condition.at (ci))) {
                        ++ei;
                        ++ci;
                }

                // Wtem w "condition natrafiamy na '%'
                if (ci < condition.size () && condition.at (ci) == '%') {
                        resume = true;
                        ++ci;

                        if (ci >= condition.size ()) {
                                return true;
                        }

                        do {
                                while (ei < event.size () && (event.at (ei) != condition.at (ci))) {
                                        ++ei;
                                }

                                // Skończyło się wejście.
                                if (ei >= event.size ()) {
                                        return false;
                                }

                                size_t ci1 = ci;
                                while (ci1 < condition.size () && condition.at (ci1) != '%' && condition.at (ci1) != '_'
                                       && (event.at (ei) == condition.at (ci1))) {
                                        ++ei;
                                        ++ci1;
                                }

                                if (ci1 < condition.size () && condition.at (ci1) != '%' && condition.at (ci1) != '_') {
                                        continue;
                                }
                                else {
                                        ci = ci1;
                                        break;
                                }

                        } while (true);
                }

        } while (resume);

        return (ci == condition.size ()) && (ei == event.size ());
}

/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE>
LikeCondition<EventT> *like (const char *condition, StripInput stripInput = StripInput::STRIP,
                             InputRetention retainInput = InputRetention::IGNORE_INPUT)
{
        return new LikeCondition<EventT> (condition, stripInput, retainInput);
}

#endif // LikeCondition_H
