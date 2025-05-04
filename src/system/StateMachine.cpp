#include "system/StateMachine.h" // Updated path
#include "states/IdleState.h"
#include "states/HomingState.h"
#include "states/PaintingState.h"
#include "states/CleaningState.h"
#include "states/PausedState.h"
#include "states/PnPState.h"
#include <Arduino.h>
#include "system/machine_state.h" // Updated path
#include "states/State.h"
#include "states/ManualMoveState.h"

//* ************************************************************************
//* ************************* STATE MACHINE *******************************
//* ************************************************************************

// Global state machine pointer
StateMachine* stateMachine = nullptr;

// Flag to prevent circular state changes
bool inStateTransition = false;

StateMachine::StateMachine() : 
    currentState(nullptr)
    // Initialize state instances here if using composition
    // homingState(), // Example - PnP was likely here
    // paintingSide1State(), 
    // ... etc.
{
    // Initialize all state objects
    idleState = new IdleState();
    homingState = new HomingState();
    paintingState = new PaintingState();
    cleaningState = new CleaningState();
    pausedState = new PausedState();
    pnpState = new PnPState();
    manualMoveState = new ManualMoveState();
    
    // Set initial state to idle
    currentState = idleState;
    currentState->enter();
    
    // Set the global pointer
    stateMachine = this;
    
    Serial.println("State Machine initialized with Idle state");
}

StateMachine::~StateMachine() {
    // Clean up all state objects
    delete idleState;
    delete homingState;
    delete paintingState;
    delete cleaningState;
    delete pausedState;
    delete pnpState;
    delete manualMoveState;
    
    // Clear the global pointer
    stateMachine = nullptr;
}

void StateMachine::changeState(State* newState) {
    if (newState == nullptr) {
        Serial.println("ERROR: Attempted to change to NULL state!");
        return;
    }
    
    // Check if already in the target state
    if (currentState == newState) {
        Serial.print("INFO: Already in state: ");
        Serial.println(getStateName(newState));
        return;
    }

    // Set flag to prevent circular state changes
    inStateTransition = true;
    
    if (currentState != nullptr) {
        Serial.print("Changing state from ");
        Serial.print(getStateName(currentState));
        Serial.print(" to ");
        Serial.println(getStateName(newState));
        currentState->exit();
    }
    
    currentState = newState;
    currentState->enter();
    
    // Clear flag
    inStateTransition = false;
}

void StateMachine::update() {
    // Update current state
    if (currentState != nullptr) {
        currentState->update();
    }
}

// Helper function to get state name for debugging
const char* StateMachine::getStateName(State* state) {
    if (state == idleState) return "Idle";
    else if (state == homingState) return "Homing";
    else if (state == paintingState) return "Painting";
    else if (state == cleaningState) return "Cleaning";
    else if (state == pausedState) return "Paused";
    else if (state == pnpState) return "PnP";
    else if (state == manualMoveState) return "Manual Move";
    else return "Unknown";
}

/* // REMOVING this duplicate/incorrect initialize() definition
void StateMachine::initialize() {
    Serial.println("Initializing State Machine...");
    // Initialize state instances - Ensure PnP is removed if it was here
    // pnpState = new PnPState(); // Example - REMOVED

    // Set the initial state
    currentState = getHomingState(); // Start with homing or idle
    if (currentState != nullptr) {
        currentState->enter();
    } else {
        Serial.println("ERROR: Initial state is null!");
        // Handle error appropriately, maybe enter an error state
    }
} 
*/ 