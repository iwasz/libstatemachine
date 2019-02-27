#include "Action.h"
#include "ErrorHandler.h"

/*****************************************************************************/

bool AndAction::run (EventType const &event)
{
        if (!current) {
                current = a;
        }

        if (current == a) {
                if (!a->run (event)) {
                        return false;
                }
                else {
                        current = b;
                }
        }

        if (current == b) {
                if (!b->run (event)) {
                        return false;
                }
                else {
                        current = nullptr;
                }
        }

        return true;
}

AndAction *and_action (Action *a, Action *b) { return new AndAction (a, b); }
