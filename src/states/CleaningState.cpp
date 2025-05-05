#include "states/CleaningState.h"
#include <Arduino.h>
#include "motors/XYZ_Movements.h"
#include "motors/ServoMotor.h"
// #include "motors/servo_control.h" // Servo control removed
#include "utils/settings.h"
#include "system/machine_state.h"
#include "hardware/paintGun_Functions.h"
#include "hardware/pressurePot_Functions.h"
// #include "hardware/Brush_Functions.h" // File does not exist

// External variable for pressure pot state
extern bool isPressurePot_ON;
// External functions for pressure pot control
extern void PressurePot_ON();
extern void PressurePot_OFF();

// External servo instance
extern ServoMotor myServo;

// Cleaning state variables
bool cleaningInProgress = false;
bool cleaningCompleted = false;
unsigned long cleaningStartTime = 0;
int cleaningStep = 0;

// Movement speed settings for cleaning
const unsigned int CLEANING_X_SPEED = 15000; // Customize these values as needed
const unsigned int CLEANING_Y_SPEED = 15000;
const unsigned int CLEANING_Z_SPEED = 4000;

CleaningState::CleaningState() {
    // Constructor implementation
}

void CleaningState::enter() {
    Serial.println("Entering Cleaning State");
    setMachineState(MachineState::CLEANING);
    
    //! Set Servo to cleaning angle
    myServo.setAngle(35);
    Serial.println("Servo set to cleaning angle (35 degrees)");
    
    // Reset cleaning state variables (keep these if used internally)
    cleaningInProgress = false;
    cleaningCompleted = false;
    cleaningStep = 0;

    // Start the cleaning cycle (this seems to be blocking)
    Serial.println("Executing Cleaning Cycle...");
    
    //! Step 1: Turn on pressure pot and initialize
    PressurePot_ON();
    delay(500); 

    //! Step 3: Move to clean station
    long cleaningX = 0.0 * STEPS_PER_INCH_XYZ;
    long cleaningY = 2.0 * STEPS_PER_INCH_XYZ;
    long cleaningZ = -2.5 * STEPS_PER_INCH_XYZ;
    moveToXYZ(cleaningX, CLEANING_X_SPEED, cleaningY, CLEANING_Y_SPEED, cleaningZ, CLEANING_Z_SPEED);
    
    //! Step 4: Activate paint gun for 1 second
    Serial.println("Activating paint gun...");
    paintGun_ON();
    delay(1000);
    paintGun_OFF();
    
    //! Step 5: Return to home position
    Serial.println("Returning to home position...");
    // Retract the paint gun
    moveToXYZ(cleaningX, CLEANING_X_SPEED, cleaningY, CLEANING_Y_SPEED, 0, CLEANING_Z_SPEED);
    // Move back to home position
    moveToXYZ(0, CLEANING_X_SPEED, 0, CLEANING_Y_SPEED, 0, CLEANING_Z_SPEED);
    
    //! Step 6: Complete cleaning cycle
    Serial.println("Cleaning Cycle Complete.");
    // cleaningCompleted = true; // Flag might not be needed if transitioning immediately
    // cleaningInProgress = false;
    
    // Reset machine state - Transition back to Idle via StateMachine
    // clearMachineState(); // REMOVE THIS - Let state machine handle it
    if (stateMachine) {
        Serial.println("Cleaning complete. Transitioning back to Idle State.");
        stateMachine->changeState(stateMachine->getIdleState());
    } else {
        Serial.println("ERROR: StateMachine pointer null in CleaningState! Cannot transition.");
        // Attempt fallback
        setMachineState(MachineState::IDLE);
    }
}

void CleaningState::update() {
    // Since enter() is blocking and transitions at the end, update() is likely unused.
}

void CleaningState::exit() {
    Serial.println("Exiting Cleaning State");
    setMachineState(MachineState::UNKNOWN); // Or IDLE?
    // Stop cleaning cycle, turn off pumps/valves, etc.
}

const char* CleaningState::getName() const {
    return "CLEANING";
}

// REMOVED Banner comment from the end of the file to avoid parsing issues.
// It should ideally be placed after includes.