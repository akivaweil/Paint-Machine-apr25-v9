#ifndef HOMING_STATE_H
#define HOMING_STATE_H

#include "State.h"
#include <FastAccelStepper.h>

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
    const char* getName() const override;

private:
    // Removed private member variables as they weren't used/initialized correctly
    // FastAccelStepperEngine* _engine = nullptr;
    // FastAccelStepper* _stepperX = nullptr;
    // ...
};

#endif // HOMING_STATE_H 