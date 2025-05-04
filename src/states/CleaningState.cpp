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
    
    // Set machine state
    setMachineState(MACHINE_CLEANING);
    
    //! Set Servo to cleaning angle
    myServo.setAngle(35);
    Serial.println("Servo set to cleaning angle (35 degrees)");
    
    // Reset cleaning state variables
    cleaningInProgress = false;
    cleaningCompleted = false;
    cleaningStep = 0;
}

void CleaningState::update() {
    // Only start cleaning once
    if (!cleaningInProgress && !cleaningCompleted) {
        cleaningInProgress = true;
        cleaningStartTime = millis();
        Serial.println("Executing Cleaning Cycle...");
        
        //! Step 1: Turn on pressure pot and initialize
        // Turn on pressure pot
        PressurePot_ON();
        delay(500);  // Short delay to let pressure build

        // Move to step 2 without using servo
        Serial.println("Cleaning Step 1 Complete - Skipped servo positioning");
        cleaningStep = 3; // Skip the servo positioning step

        //! Step 3: Move to clean station
        // Move to cleaning station position
        // Convert inches to steps for the cleaning position (5, 10, -2.5)
        long cleaningX = 0.0 * STEPS_PER_INCH_XYZ;
        long cleaningY = 2.0 * STEPS_PER_INCH_XYZ;
        long cleaningZ = -2.5 * STEPS_PER_INCH_XYZ;
        
        // Move to cleaning position
        moveToXYZ(cleaningX, CLEANING_X_SPEED, cleaningY, CLEANING_Y_SPEED, cleaningZ, CLEANING_Z_SPEED);
        
        //! Step 4: Activate paint gun for 1 second
        Serial.println("Activating paint gun...");
        paintGun_ON();
        delay(1000);
        paintGun_OFF();
        
        //! Step 5: Return to home position
        Serial.println("Returning to home position...");

        //Retract the paint gun
        moveToXYZ(cleaningX, CLEANING_X_SPEED, cleaningY, CLEANING_Y_SPEED, 0, CLEANING_Z_SPEED);

        // Move back to home position
        moveToXYZ(0, CLEANING_X_SPEED, 0, CLEANING_Y_SPEED, 0, CLEANING_Z_SPEED);
        
        //! Step 6: Complete cleaning cycle
        Serial.println("Cleaning Cycle Complete.");
        cleaningCompleted = true;
        cleaningInProgress = false;
        
        // Reset machine state
        clearMachineState();
    }
}

void CleaningState::exit() {
    Serial.println("Exiting Cleaning State");
    
    // Ensure paint gun is off when exiting
    paintGun_OFF();
    
    // Reset machine state
    clearMachineState();
    
    // Reset cleaning state variables
    cleaningInProgress = false;
    cleaningCompleted = false;
    cleaningStep = 0;
} 