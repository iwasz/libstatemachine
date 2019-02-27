#ifndef STATE_MACHINE_STATE_H
#define STATE_MACHINE_STATE_H

#include <cstdint>
class Transition;
class Action;

/**
 * @brief Stan. Uwaga, label nie może być 0.
 */
class State {
public:
        /*
         * pola bitowe
         */
        enum Flags {
                NONE = 0x00,
                INITIAL = 0x01,
                INC_SYNCHRO = 0x02,
                SUPPRESS_GLOBAL_TRANSITIONS = 0x04
        };

        /**
         * @brief State
         * @param label Służy do odróżniania stanów (nie są rozróżniane po wskaźnikach). Nie może być 0, ale nie ma sprawdzenia!
         * @param entryAction
         * @param exitAction
         */
        State (uint8_t label, Action *entryAction = nullptr, Action *exitAction = nullptr)
            : entryAction (entryAction),
              exitAction (exitAction),
              label (label),
              firstTransition (nullptr),
              lastAddedTransition (nullptr),
              flags (NONE)
        {
        }

        Action *getEntryAction () { return entryAction; }
        void setEntryAction (Action *a) { entryAction = a; }

        Action *getExitAction () { return exitAction; }
        void setExitAction (Action *a) { exitAction = a; }

        uint8_t getLabel () const { return label; }

        void addTransition (Transition *t);
        Transition *getFirstTransition () { return firstTransition; }

        uint8_t getFlags () const { return flags; }
        void setFlags (uint8_t value) { flags = value; }

#ifndef UNIT_TEST
private:
#endif

        Action *entryAction;
        Action *exitAction;
        uint8_t label;
        Transition *firstTransition;
        Transition *lastAddedTransition;
        uint8_t flags;
};

#endif // STATE_H
