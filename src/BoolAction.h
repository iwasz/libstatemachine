/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef BOOLACTION_H
#define BOOLACTION_H

#include "Action.h"
#include "DelayAction.h"
#include <cstdint>

/**
 * @brief Akcja, która przypisuje jednej zmiennej bool drugą zmienną bool.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class BoolAction : public Action<EventT> {
public:
        using EventType = EventT;

        /**
         * @brief Akcja wysyłająca komendę do modemu GSM.
         * @param c Komenda.
         */
        BoolAction (bool *destination, bool *source) : destination (destination), source (source) {}
        virtual ~BoolAction () {}

        virtual bool run (EventType const &)
        {
                *destination = *source;
                return true;
        }

private:
        bool *destination;
        bool *source;
};

#endif // BOOLACTION_H
