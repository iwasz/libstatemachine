#ifndef GSMCOMMANDACTION_H
#define GSMCOMMANDACTION_H

#include <Action.h>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

extern std::vector<std::string> gsmModemCommandsIssued;

/**
 * @brief Wysyłajakieś polecenie AT do modemu.
 */
template <typename EventT = string> class GsmCommandAction : public Action<EventT> {
public:
        using EventType = EventT;

        GsmCommandAction (const char *c) : command (c), len (0) {}
        GsmCommandAction (uint8_t const *c, uint16_t len) : command (reinterpret_cast<const char *> (c)), len (len) {}
        virtual ~GsmCommandAction () = default;

        bool run (EventType const &event);

protected:
        const char *command;
        uint16_t len;
};

/*****************************************************************************/

template <typename EventT> bool GsmCommandAction<EventT>::run (EventType const &)
{
        gsmModemCommandsIssued.push_back (command);
        return true;
}

/*****************************************************************************/

template <typename EventT = string> GsmCommandAction<EventT> *gsm (const char *command) { return new GsmCommandAction<EventT> (command); }

/*****************************************************************************/

template <typename EventT = string> GsmCommandAction<EventT> *gsmBin (uint8_t const *command, uint16_t len)
{
        return new GsmCommandAction<EventT> (command, len);
}

#endif // GSMCOMMANDACTION_H
