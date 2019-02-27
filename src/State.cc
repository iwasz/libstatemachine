#include "State.h"
#include "Action.h"
#include "Transition.h"

void State::addTransition (Transition *t)
{
        if (!lastAddedTransition) {
                firstTransition = lastAddedTransition = t;
        }
        else {
                lastAddedTransition->next = t;
                lastAddedTransition = t;
        }
}
