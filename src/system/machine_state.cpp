#include <Arduino.h>
#include "system/machine_state.h" // Updated path
#include "system/StateMachine.h"  // Updated path

//* ************************************************************************
//* ************************ MACHINE STATE ****************************
//* ************************************************************************

// Current machine state
volatile int currentMachineState = MACHINE_IDLE;

// Machine flags
volatile bool homeCommandReceived = false;

// Set the machine state
void setMachineState(int state) {
    currentMachineState = state;
    
    // Debug output
    Serial.print("Machine state set to: ");
    switch (state) {
        case MACHINE_IDLE:
            Serial.println("IDLE");
            // Sync with state machine if it exists
            syncMachineStateWithStateMachine(MACHINE_IDLE);
            break;
        case MACHINE_HOMING:
            Serial.println("HOMING");
            syncMachineStateWithStateMachine(MACHINE_HOMING);
            break;
        case MACHINE_PAINTING:
            Serial.println("PAINTING");
            syncMachineStateWithStateMachine(MACHINE_PAINTING);
            break;
        case MACHINE_PNP:
            Serial.println("PNP");
            syncMachineStateWithStateMachine(MACHINE_PNP);
            break;
        case MACHINE_CLEANING:
            Serial.println("CLEANING");
            syncMachineStateWithStateMachine(MACHINE_CLEANING);
            break;
        case MACHINE_MANUAL_MOVE:
            Serial.println("MANUAL MOVE");
            syncMachineStateWithStateMachine(MACHINE_MANUAL_MOVE);
            break;
        case MACHINE_ERROR:
            Serial.println("ERROR");
            syncMachineStateWithStateMachine(MACHINE_ERROR);
            break;
        default:
            Serial.println("UNKNOWN");
            break;
    }
}

// Clear the machine state (set to idle)
void clearMachineState() {
    currentMachineState = MACHINE_IDLE;
    Serial.println("Machine state cleared to IDLE");
    
    // Sync with state machine if it exists
    syncMachineStateWithStateMachine(MACHINE_IDLE);
}

// Get the current machine state
int getMachineState() {
    return currentMachineState;
}

// Function to sync the numeric machine state with the object-oriented StateMachine
void syncMachineStateWithStateMachine(int state) {
    // Only sync if stateMachine exists and if we're not already in a state transition
    // Use the flag defined within StateMachine.cpp if accessible, otherwise redefine locally.
    // Assuming 'inStateTransition' is a global or accessible flag from StateMachine.cpp
    extern bool inStateTransition;
    
    if (stateMachine && !inStateTransition) {
        // The changeState method in StateMachine now handles the inStateTransition flag internally.
        // We don't need to manage it here.
        
        State* currentState = stateMachine->getCurrentState();
        State* targetState = nullptr;
        
        switch(state) {
            case MACHINE_IDLE:
                targetState = stateMachine->getIdleState();
                break;
            case MACHINE_HOMING:
                targetState = stateMachine->getHomingState();
                break;
            case MACHINE_PAINTING:
                targetState = stateMachine->getPaintingState();
                break;
            case MACHINE_PNP:
                // targetState = stateMachine->getPnpState(); // Removed - PnP is now procedural
                // Serial.println("SYNC: State 3 (Old PnP) requested, but PnP is now procedural. Ignoring state sync.");
                targetState = stateMachine->getPnpState(); // Re-enable PnP state sync
                break;
            case MACHINE_CLEANING:
                targetState = stateMachine->getCleaningState();
                break;
            case MACHINE_MANUAL_MOVE:
                targetState = stateMachine->getManualMoveState();
                break;
            // Add more states as needed
             case MACHINE_ERROR: // Add error state handling if needed
                 // targetState = stateMachine->getErrorState(); // Assuming an ErrorState exists
                 Serial.println("SYNC: Error state requested, no specific error state object defined yet.");
                 break;
             default:
                Serial.printf("SYNC: Unknown machine state %d requested.\n", state);
                break;
        }
        
        // Change to target state if it's different from current state and not null
        if (targetState && targetState != currentState) {
            Serial.println("Syncing numeric state with StateMachine object");
            stateMachine->changeState(targetState);
        } else if (targetState == currentState) {
            // Serial.println("SYNC: Already in target state."); // Optional debug msg
        } else if (!targetState && state != MACHINE_ERROR && state != MACHINE_PNP) { 
            // Only warn if targetState is null for states that *should* have an object (excluding PnP/Error for now)
            Serial.printf("SYNC: Could not find StateMachine object for state %d\n", state);
        }
        
        // inStateTransition flag is now handled by changeState
    }
}

// Update machine state based on current conditions
void updateMachineState() {
    // This function is called every loop and processes state transitions
    // based on the current machine state and conditions
    
    // Check for home command received flag
    if (homeCommandReceived) {
        setMachineState(MACHINE_HOMING);
        homeCommandReceived = false;  // Reset the flag
    }
    
    switch (currentMachineState) {
        case MACHINE_IDLE:
            // Process idle state logic
            break;
            
        case MACHINE_HOMING:
            // Process homing state logic
            break;
            
        case MACHINE_PAINTING:
            // Process painting state logic
            break;
            
        case MACHINE_PNP:
            // Process pick and place state logic
            break;
            
        case MACHINE_CLEANING:
            // Process cleaning state logic
            break;
            
        case MACHINE_MANUAL_MOVE:
            // Process manual move logic (mostly handled by state object via WebSocket)
            break;

        case MACHINE_ERROR:
            // Process error state logic
            break;
    }
} 