#include "states/PaintingState.h"
#include <Arduino.h>
#include "motors/PaintingSides.h" // Include header for paintAllSides
#include "system/StateMachine.h"  // Include header for StateMachine access
// #include "motors/Homing.h"        // REMOVE: Homing will be handled by HomingState
#include <FastAccelStepper.h>      // Include for stepper access
#include "persistence/PaintingSettings.h"
#include "motors/Rotation_Motor.h"
// #include "system/machine_state.h" // No longer needed
#include "hardware/paintGun_Functions.h" // Added include for paintGun_OFF
#include "motors/XYZ_Movements.h"      // ADDED: For moveToXYZ
#include "utils/settings.h"            // ADDED: For default speeds

// Define necessary variables or includes specific to PaintingState if known
// #include "settings.h"
// #include "XYZ_Movements.h"

extern StateMachine *stateMachine; // Access the global state machine instance

// Need access to the global stepper instances and engine
extern FastAccelStepperEngine engine;
extern FastAccelStepper* stepperX;
extern FastAccelStepper* stepperY_Left;
extern FastAccelStepper* stepperY_Right;
extern FastAccelStepper* stepperZ;

//* ************************************************************************
//* ************************* PAINTING STATE ***************************
//* ************************************************************************

PaintingState::PaintingState() {
    // Constructor implementation
}

void PaintingState::enter() {
    Serial.println("Entering Painting State (All Sides)");
    // setMachineState(MachineState::PAINTING); // REMOVED
    
    paintAllSides(); // Call the function to paint all sides

    // After painting is complete, move to 0,0,0 then transition to HomingState
    Serial.println("All Sides Painting complete. Moving to 0,0,0 before transitioning to Homing State...");
    moveToXYZ(0, DEFAULT_X_SPEED, 0, DEFAULT_Y_SPEED, 0, DEFAULT_Z_SPEED); 
    
    if (stateMachine) {
        stateMachine->changeState(stateMachine->getHomingState());
    } else {
        Serial.println("Error: StateMachine pointer is null in PaintingState::enter(). Cannot transition to Homing.");
        // Attempt to go to Idle as a fallback if HomingState transition fails,
        // though this part might not be reached if stateMachine is null.
        if (stateMachine && stateMachine->getIdleState()) { // Check if getIdleState() is valid
             stateMachine->changeState(stateMachine->getIdleState());
        } else {
            Serial.println("Error: Cannot transition to IdleState either.");
        }
    }
}

void PaintingState::update() {
    // Code to run repeatedly during the painting state
    // This state now completes its work in enter(), so update might be empty
    // Or it could monitor for abort commands if needed in the future.
}

void PaintingState::exit() {
    Serial.println("Exiting Painting State (All Sides)");
    // setMachineState(MachineState::UNKNOWN); // REMOVED
    // Stop paint gun, ensure motors are stopped, etc.
    paintGun_OFF(); 
}

const char* PaintingState::getName() const {
    return "PAINTING";
}

//* ************************************************************************
//* ************************** PAINTING STATE ****************************
//* ************************************************************************ 