#include <Arduino.h>
#include "motors/PaintingSides.h"
#include "../../include/web/Web_Dashboard_Commands.h" // For checkForHomeCommand
#include "motors/ServoMotor.h" // Added for cleaning burst
#include "hardware/paintGun_Functions.h" // Added for cleaning burst
#include "hardware/pressurePot_Functions.h" // Added for pressure pot check
#include "motors/XYZ_Movements.h" // Added for cleaning movement
#include "utils/settings.h"     // Added for STEPS_PER_INCH_XYZ and speeds
#include <FastAccelStepper.h>    // Added for stepper extern declarations

extern ServoMotor myServo; // Added for cleaning burst
extern FastAccelStepper *stepperX;      // Added for Z move
extern FastAccelStepper *stepperY_Left; // Added for Z move
extern FastAccelStepper *stepperZ;      // Added for Z move
extern bool isPressurePot_ON; // Added for pressure pot check

//* ************************************************************************
//* ********************** ALL SIDES PAINTING ************************
//* ************************************************************************

// Cleaning parameters
const float CLEANING_X_INCH = 0.0;
const float CLEANING_Y_INCH = 2.0;
const float CLEANING_Z_INCH = -2.5;
const unsigned int CLEANING_X_SPEED = 15000;
const unsigned int CLEANING_Y_SPEED = 15000;
const unsigned int CLEANING_Z_SPEED = 4000;

// Function to paint all sides in sequence
void paintAllSides() {
    Serial.println("Starting All Sides Painting Sequence");

    //! Check if pressure pot needs to be turned on
    if (!isPressurePot_ON) {
        Serial.println("Pressure pot is off. Turning on and pressurizing for 1 second...");
        PressurePot_ON();
        delay(1000); // Wait 1 second for pressure build-up
        Serial.println("Pressurization complete.");
    } else {
        Serial.println("Pressure pot already on.");
    }
    
    //! Move to cleaning position and perform short burst
    Serial.println("Moving to cleaning position...");
    long cleaningX_steps = (long)(CLEANING_X_INCH * STEPS_PER_INCH_XYZ);
    long cleaningY_steps = (long)(CLEANING_Y_INCH * STEPS_PER_INCH_XYZ);
    long cleaningZ_steps = (long)(CLEANING_Z_INCH * STEPS_PER_INCH_XYZ);

    myServo.setAngle(35); // Set servo to cleaning angle
    moveToXYZ(cleaningX_steps, CLEANING_X_SPEED, cleaningY_steps, CLEANING_Y_SPEED, cleaningZ_steps, CLEANING_Z_SPEED);

    Serial.println("Starting short cleaning burst...");
    paintGun_ON();
    delay(500); // Keep paint gun on for 0.5 seconds
    paintGun_OFF();
    Serial.println("Cleaning burst complete.");
    // Note: Servo remains at cleaning angle, assuming painting functions will set their required angles. If not, add code here to return servo to a default angle.
    
    //! Move Z axis to home (0) before starting paint sequence
    Serial.println("Moving Z axis to home (0)...");
    // Use current X/Y positions from after the cleaning move
    moveToXYZ(stepperX->getCurrentPosition(), CLEANING_X_SPEED, stepperY_Left->getCurrentPosition(), CLEANING_Y_SPEED, 0, CLEANING_Z_SPEED);
    Serial.println("Z axis at 0. Starting painting sequence.");
    // Note: Servo remains at cleaning angle, assuming painting functions will set their required angles.
    
    //! STEP 1: Paint front side
    Serial.println("Starting Front Side");
    paintSide1Pattern();
    
    // Check for home command between sides
    if (checkForHomeCommand()) {
        Serial.println("All Sides Painting ABORTED due to home command after front side");
        return;
    }
    
    //! STEP 2: Paint back side
    Serial.println("Starting Back Side");
    paintSide3Pattern();
    
    // Check for home command between sides
    if (checkForHomeCommand()) {
        Serial.println("All Sides Painting ABORTED due to home command after back side");
        return;
    }
    
    //! STEP 3: Paint left side
    Serial.println("Starting Left Side");
    paintSide4Pattern();
    
    // Check for home command between sides
    if (checkForHomeCommand()) {
        Serial.println("All Sides Painting ABORTED due to home command after left side");
        return;
    }
    
    //! STEP 4: Paint right side
    Serial.println("Starting Right Side");
    paintSide2Pattern();
    
    Serial.println("All Sides Painting Sequence Completed");
} 