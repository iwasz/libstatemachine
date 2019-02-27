#ifndef GSMCOMMANDACTION_H
#define GSMCOMMANDACTION_H

#include <cstdint>
#include <Action.h>
#include <vector>
#include <string>

extern std::vector<std::string> gsmModemCommandsIssued;

/**
 * @brief Wysyłajakieś polecenie AT do modemu.
 */
class GsmCommandAction : public Action {
public:
        /**
         * @brief Akcja wysyłająca komendę do modemu GSM.
         * @param c Komenda.
         */
        GsmCommandAction (const char *c) : command (c), len (0) {}
        GsmCommandAction (uint8_t const *c, uint16_t len) : command (reinterpret_cast <const char *> (c)), len (len) {}
        virtual ~GsmCommandAction () {}
        virtual bool run (const char *input);

protected:
        const char *command;
        uint16_t len;
};

extern GsmCommandAction *gsm (const char *command);
extern GsmCommandAction *gsmBin (uint8_t const *command, uint16_t len);

#endif // GSMCOMMANDACTION_H

