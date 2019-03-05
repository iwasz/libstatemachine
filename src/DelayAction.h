/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef DELAYACTION_H
#define DELAYACTION_H

#include "Action.h"
#include "Timer.h"
#include <cstdint>
#ifdef UNIT_TEST
#include <cstdio>
#endif

/**
 * @brief Akcja, której jedynym zadaniem jest opóźnienie. Zatrzymuje całą maszynę i następna
 * akcja poniej, albo następne przejście wykona się dopiero po upłynięciu zadanego czasu.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class DelayAction : public Action<EventT> {
public:
        using EventType = EventT;

        /**
         * @brief Akcja wysyłająca komendę do modemu GSM.
         * @param c Komenda.
         */
        DelayAction (uint16_t delayMs) : lastMs (0), delayMs (delayMs) {}
        virtual ~DelayAction () = default;

        virtual bool run (EventType const &event);

private:
        uint32_t lastMs;
        uint16_t delayMs;
};

/*****************************************************************************/

template <typename EventT> bool DelayAction<EventT>::run (const EventType &event)
{
        if (!lastMs) {
                lastMs = Timer::getTick ();
        }

        uint32_t currentMs = Timer::getTick ();

            if (currentMs - lastMs >= delayMs) {
                lastMs = 0;

#if 0 && !defined(UNIT_TEST)
        printf ("delay %dms\n", delayMs);
#endif

                return true;
        }

        return false;
}

/**
 * @brief Helper do tworzenia opóźnień.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> DelayAction<EventT> *delayMs (uint16_t delay) { return new DelayAction<EventT> (delay); }

#endif // DELAYACTION_H
