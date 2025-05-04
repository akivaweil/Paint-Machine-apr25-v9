#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "states/State.h"          // Base State class
#include "states/HomingState.h"
#include "states/PaintingState.h"
#include "states/CleaningState.h"
#include "states/PausedState.h"
#include "states/IdleState.h"
#include "states/PnPState.h"
#include "states/ManualMoveState.h"

class StateMachine {
public:
    StateMachine();
    ~StateMachine();
    
    void changeState(State* newState);
    void update();
    State* getCurrentState() { return currentState; }
    
    // Getter methods for state access from other classes
    State* getIdleState() { return idleState; }
    State* getHomingState() { return homingState; }
    State* getPaintingState() { return paintingState; }
    State* getCleaningState() { return cleaningState; }
    State* getPausedState() { return pausedState; }
    State* getPnpState() { return pnpState; }
    State* getManualMoveState() { return manualMoveState; }
    
    // Helper method to get state name for debugging
    const char* getStateName(State* state);

private:
    State* currentState;
    State* idleState;
    State* homingState;
    State* paintingState;
    State* cleaningState;
    State* pausedState;
    State* pnpState;
    State* manualMoveState;
};

#endif // STATEMACHINE_H 