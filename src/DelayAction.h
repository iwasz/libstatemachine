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
#include <cstdint>

/**
 * @brief Akcja, której jedynym zadaniem jest opóźnienie. Zatrzymuje całą maszynę i następna
 * akcja poniej, albo następne przejście wykona się dopiero po upłynięciu zadanego czasu.
 */
class DelayAction : public Action {
public:
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

/**
 * @brief Helper do tworzenia opóźnień.
 */
extern DelayAction *delayMs (uint16_t delay);

#endif // DELAYACTION_H
