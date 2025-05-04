#ifndef HOMING_STATE_H
#define HOMING_STATE_H

#include "State.h"

// Machine state constants
#define MACHINE_HOMING 1

// Function declarations
bool homeAllAxes();

class HomingState : public State {
public:
    HomingState();
    void enter() override;
    void update() override;
    void exit() override;
};

#endif // HOMING_STATE_H 