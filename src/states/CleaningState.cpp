#include "states/CleaningState.h"
#include <Arduino.h>
#include "motors/XYZ_Movements.h"
#include "motors/ServoMotor.h"
// #include "motors/servo_control.h" // Servo control removed
#include "utils/settings.h"
// #include "system/machine_state.h" // No longer needed
#include "hardware/paintGun_Functions.h"
#include "hardware/pressurePot_Functions.h"
#include "system/StateMachine.h" // Added include
// #include "hardware/Brush_Functions.h" // File does not exist

// External variable for pressure pot state
extern bool isPressurePot_ON;
// External functions for pressure pot control
extern void PressurePot_ON();
extern void PressurePot_OFF();

// External servo instance
extern ServoMotor myServo;

// Reference to the global state machine instance
extern StateMachine* stateMachine;

// Cleaning state variables
bool cleaningInProgress = false;
bool cleaningCompleted = false;
unsigned long cleaningStartTime = 0;
int cleaningStep = 0;

// Movement speed settings for cleaning
const unsigned int CLEANING_X_SPEED = 15000; // Customize these values as needed
const unsigned int CLEANING_Y_SPEED = 15000;
const unsigned int CLEANING_Z_SPEED = 4000;

CleaningState::CleaningState() : 
    _isCleaning(false),
    _cleaningComplete(false)
{
    // Constructor implementation
}

void CleaningState::enter() {
    Serial.println("Entering Cleaning State");
    
    //! Set Servo to cleaning angle
    myServo.setAngle(35);
    Serial.println("Servo set to cleaning angle (35 degrees)");
    
    // Reset cleaning state variables
    _isCleaning = true;
    _cleaningComplete = false;
    Serial.println("Cleaning process initiated...");
    // DO NOT execute blocking cycle here
}

void CleaningState::update() {
    // If cleaning is active and not yet complete
    if (_isCleaning && !_cleaningComplete) {
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
        
        // Mark cleaning as complete
        _cleaningComplete = true;
        _isCleaning = false;
    }
    
    // If cleaning is marked as complete, transition back to Idle
    if (_cleaningComplete) {
        if (stateMachine) {
            Serial.println("Cleaning complete. Transitioning back to Idle State.");
            stateMachine->changeState(stateMachine->getIdleState());
             // Reset flag for next entry after transition
            _cleaningComplete = false; 
        } else {
            Serial.println("ERROR: StateMachine pointer null in CleaningState! Cannot transition.");
            // Prevent potential infinite loop if stateMachine is null
            _cleaningComplete = false; 
        }
    }
}

void CleaningState::exit() {
    Serial.println("Exiting Cleaning State");
    // Ensure pressure pot is off when exiting, regardless of cycle completion state
    PressurePot_OFF(); 
    _isCleaning = false; // Ensure flags are reset
    _cleaningComplete = false;
}

const char* CleaningState::getName() const {
    return "CLEANING";
}

// REMOVED Banner comment from the end of the file to avoid parsing issues.
// It should ideally be placed after includes.