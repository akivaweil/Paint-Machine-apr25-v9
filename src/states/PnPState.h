#pragma once

#include <Bounce2.h>
#include <FastAccelStepper.h>
#include "states/State.h" // Assuming a base State class exists
#include "utils/settings.h" // For grid dimensions, pins etc.

// Assuming StateMachine is needed for transitions
class StateMachine; 
extern StateMachine* stateMachine; 

class PnPState : public State {
public:
    PnPState();
    void enter() override;
    void update() override;
    void exit() override;

private:
    // Grid configuration
    float gridPositionsX[GRID_ROWS * GRID_COLS];
    float gridPositionsY[GRID_ROWS * GRID_COLS];

    // Position tracking
    int currentPnPGridPosition;
    bool pnpCycleIsComplete;

    // Standby position (in steps)
    long pnpStandbyX_steps;
    long pnpStandbyY_steps;

    // Cycle switch
    Bounce pnpCycleSwitchDebouncer;
    bool previousSwitchState; // To detect rising edges (new presses)
    
    // Cycle timeout
    unsigned long lastCycleTime;  // Timestamp of the last cycle completion
    static const unsigned long CYCLE_TIMEOUT = 1000; // 1 second timeout between cycles

    // Simplified state tracking within PnPState
    // 0: Initializing / Moving to Standby first time
    // 1: Waiting at Standby
    // 2: Currently processing a PnP cycle (blocking) - handled within update
    // 3: Moving back to Standby (non-blocking)
    // 4: PnP Complete, ready for homing transition
    int pnp_step; 

    // Flag to signal that homing is needed after PnP completion
    bool homingNeededAfterPnP; 

    // Private helper methods
    void calculateGridPositions();
    void initializeHardware();
    void moveToStandby(bool initialMove = false);
    void process_single_pnp_cycle(); 

    // Stepper references (assuming they are globally accessible or passed somehow)
    // If they are global like in pnp_State.cpp, we might not need them as members.
    // extern FastAccelStepperEngine engine; // Example if needed
    // extern FastAccelStepper *stepperX, *stepperY_Left, *stepperY_Right; // Example
}; 