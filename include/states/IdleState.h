#ifndef IDLE_STATE_H
#define IDLE_STATE_H

#include "State.h"
#include <Bounce2.h> // Added for button debouncing

// Machine state constants
#define MACHINE_IDLE 0

class IdleState : public State {
public:
    IdleState();
    void enter() override;
    void update() override;
    void exit() override;

private:
    Bounce pnpCycleSwitch; // Declare the debouncer object
};

#endif // IDLE_STATE_H 