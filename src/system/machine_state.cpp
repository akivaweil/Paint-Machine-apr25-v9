#include <Arduino.h>
#include "system/machine_state.h" // Updated path
#include "system/StateMachine.h"  // Updated path

//* ************************************************************************
//* ************************ MACHINE STATE ****************************
//* ************************************************************************

// Current machine state using the enum
volatile MachineState currentMachineState = MachineState::IDLE;

// Machine flags
volatile bool homeCommandReceived = false;

// Set the machine state using the enum
void setMachineState(MachineState state) {
    currentMachineState = state;
    
    // Debug output
    Serial.print("Machine state set to: ");
    switch (state) {
        case MachineState::IDLE:
            Serial.println("IDLE");
            syncMachineStateWithStateMachine(MachineState::IDLE);
            break;
        case MachineState::HOMING:
            Serial.println("HOMING");
            syncMachineStateWithStateMachine(MachineState::HOMING);
            break;
        case MachineState::PAINTING:
            Serial.println("PAINTING");
            syncMachineStateWithStateMachine(MachineState::PAINTING);
            break;
        case MachineState::PNP:
            Serial.println("PNP");
            syncMachineStateWithStateMachine(MachineState::PNP);
            break;
        case MachineState::CLEANING:
            Serial.println("CLEANING");
            syncMachineStateWithStateMachine(MachineState::CLEANING);
            break;
        case MachineState::MANUAL:
            Serial.println("MANUAL");
            syncMachineStateWithStateMachine(MachineState::MANUAL);
            break;
        case MachineState::PAUSED:
            Serial.println("PAUSED");
            syncMachineStateWithStateMachine(MachineState::PAUSED);
            break;
        case MachineState::ERROR:
            Serial.println("ERROR");
            syncMachineStateWithStateMachine(MachineState::ERROR);
            break;
        case MachineState::UNKNOWN: // Fallthrough default
        default:
            Serial.println("UNKNOWN");
            // Don't sync UNKNOWN state?
            break;
    }
}

// Clear the machine state (set to idle or unknown?)
void clearMachineState() {
    // Let's set to IDLE as the default cleared state
    setMachineState(MachineState::IDLE);
    // Serial.println("Machine state cleared to IDLE"); // setMachineState logs this
}

// Get the current machine state (returns the enum)
MachineState getMachineState() {
    return currentMachineState;
}

// Function to sync the numeric machine state with the object-oriented StateMachine
void syncMachineStateWithStateMachine(MachineState state) { // Use enum
    extern bool inStateTransition; // Still need this check?
    
    if (stateMachine && !inStateTransition) {
        State* currentState = stateMachine->getCurrentState();
        State* targetState = nullptr;
        
        switch(state) {
            case MachineState::IDLE:
                targetState = stateMachine->getIdleState();
                break;
            case MachineState::HOMING:
                targetState = stateMachine->getHomingState();
                break;
            case MachineState::PAINTING:
                targetState = stateMachine->getPaintingState();
                break;
            case MachineState::PNP:
                targetState = stateMachine->getPnpState();
                break;
            case MachineState::CLEANING:
                targetState = stateMachine->getCleaningState();
                break;
            case MachineState::MANUAL:
                targetState = stateMachine->getManualMoveState();
                break;
            case MachineState::PAUSED:
                targetState = stateMachine->getPausedState();
                break;
            case MachineState::ERROR: 
                 Serial.println("SYNC: Error state requested, no specific error state object defined yet.");
                 break;
             case MachineState::UNKNOWN:
             default:
                // Explicitly handle UNKNOWN or default case, maybe don't sync?
                Serial.println("SYNC: UNKNOWN or invalid state requested.");
                break;
        }
        
        // Change to target state if it's different from current state and not null
        if (targetState && targetState != currentState) {
            Serial.println("Syncing machine_state enum with StateMachine object");
            stateMachine->changeState(targetState);
        } 
        // Optional logging for other cases removed for brevity
    }
}

// Update machine state based on current conditions
void updateMachineState() {
    // This function might be redundant if state transitions are primarily driven by StateMachine
    // Kept for now, but review its necessity later.
    
    // Check for home command received flag
    if (homeCommandReceived) {
        setMachineState(MachineState::HOMING);
        homeCommandReceived = false;  // Reset the flag
    }
    
    // The switch statement here likely doesn't do much if state logic is in State objects.
    // Consider removing or simplifying this.
    /*
    switch (currentMachineState) {
        case MachineState::IDLE: break;
        // ... other cases ...
    }
    */
} 