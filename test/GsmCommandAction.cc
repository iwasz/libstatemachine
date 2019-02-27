#include <cstdio>
#include "GsmCommandAction.h"

std::vector<std::string> gsmModemCommandsIssued;

bool GsmCommandAction::run (const char *, void *)
{
        gsmModemCommandsIssued.push_back (command);
        return true;
}

/*****************************************************************************/

GsmCommandAction *gsm (const char *command)
{
        return new GsmCommandAction (command);
}

/*****************************************************************************/

extern GsmCommandAction *gsmBin (uint8_t const *command, uint16_t len)
{
        return new GsmCommandAction (command, len);
}
