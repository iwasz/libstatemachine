#include "Action.h"
#include "ErrorHandler.h"

#if 0
///*****************************************************************************/

// void Action::setNext (Action *n)
//{
//        if (next) {
//                Error_Handler ();
//        }

//        next = n;
//}

///*****************************************************************************/

bool AndAction::run (const char *input)
{
        if (!current) {
                current = a;
        }

        if (current == a) {
                if (!a->run (input)) {
                        return false;
                }
                else {
                        current = b;
                }
        }

        if (current == b) {
                if (!b->run (input)) {
                        return false;
                }
                else {
                        current = nullptr;
                }
        }

        return true;
}

AndAction *and_action (Action *a, Action *b) { return new AndAction (a, b); }
#endif
