#include "states/PaintingState.h"
#include <Arduino.h>
#include "motors/PaintingSides.h" // Include header for paintAllSides
#include "system/StateMachine.h"  // Include header for StateMachine access
#include "motors/Homing.h"        // Include the Homing class header
#include <FastAccelStepper.h>      // Include for stepper access
#include "persistence/PaintingSettings.h"
#include "motors/Rotation_Motor.h"

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
    Serial.println("Entering Painting State");
    // Code to run once when entering the painting state
    
    paintAllSides(); // Call the function to paint all sides

    // Create a local Homing controller instance and home axes
    Serial.println("Painting complete. Initiating homing...");
    Homing homingController(engine, stepperX, stepperY_Left, stepperY_Right, stepperZ);
    bool homingSuccess = homingController.homeAllAxes(); // Call the member function

    // After painting and homing are complete, transition back to Idle
    if (homingSuccess) {
        Serial.println("Painting & Homing complete. Transitioning back to Idle.");
    } else {
        Serial.println("Painting complete, but Homing FAILED. Transitioning back to Idle anyway.");
        // Consider transitioning to an Error state here if you have one
    }
    
    if (stateMachine) {
        stateMachine->changeState(stateMachine->getIdleState());
    } else {
        Serial.println("Error: StateMachine pointer is null in PaintingState::enter()");
        // Handle error appropriately, maybe go to an error state or home
    }
}

void PaintingState::update() {
    // Code to run repeatedly during the painting state
    // This state now completes its work in enter(), so update might be empty
    // Or it could monitor for abort commands if needed in the future.
}

void PaintingState::exit() {
    Serial.println("Exiting Painting State");
    // Code to run once when exiting the painting state
} 