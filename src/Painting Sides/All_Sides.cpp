#include <Arduino.h>
#include "motors/PaintingSides.h"
#include "../../include/web/Web_Dashboard_Commands.h" // For checkForHomeCommand
#include "motors/ServoMotor.h" // Added for cleaning burst
#include "hardware/paintGun_Functions.h" // Added for cleaning burst
#include "hardware/pressurePot_Functions.h" // Added for pressure pot check
#include "motors/XYZ_Movements.h" // Added for cleaning movement
#include "utils/settings.h"     // Added for STEPS_PER_INCH_XYZ and speeds
#include <FastAccelStepper.h>    // Added for stepper extern declarations
#include "motors/Homing.h"      // For Homing class and homeAllAxes()

extern ServoMotor myServo; // Added for cleaning burst
extern FastAccelStepper *stepperX;      // Added for Z move
extern FastAccelStepper *stepperY_Left; // Added for Z move
extern FastAccelStepper *stepperY_Right; // Needed for Homing class constructor
extern FastAccelStepper *stepperZ;      // Added for Z move
extern bool isPressurePot_ON; // Added for pressure pot check
extern FastAccelStepperEngine engine; // Needed for Homing class constructor

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

const unsigned long ALL_SIDES_REPEAT_DELAY_MS = 30 * 1000; // 30 seconds in milliseconds

// Helper function for a single painting sequence
// Returns true if completed, false if aborted by home command
bool _executeSinglePaintAllSidesSequence(const char* runLabel) {
    Serial.print("Starting All Sides Painting Sequence (");
    Serial.print(runLabel);
    Serial.println(")");

    //! Check if pressure pot needs to be turned on
    if (!isPressurePot_ON) {
        Serial.print("Pressure pot is off. Turning on and pressurizing for 1 second... (");
        Serial.print(runLabel);
        Serial.println(")");
        PressurePot_ON();
        delay(1000); // Wait 1 second for pressure build-up
        Serial.print("Pressurization complete. (");
        Serial.print(runLabel);
        Serial.println(")");
    } else {
        Serial.print("Pressure pot already on. (");
        Serial.print(runLabel);
        Serial.println(")");
    }
    
    // --- START: Comment out redundant cleaning burst in All_Sides ---
    /*
    //! Move to cleaning position and perform short burst
    Serial.print("Moving to cleaning position... (");
    Serial.print(runLabel);
    Serial.println(")");
    long cleaningX_steps = (long)(CLEANING_X_INCH * STEPS_PER_INCH_XYZ);
    long cleaningY_steps = (long)(CLEANING_Y_INCH * STEPS_PER_INCH_XYZ);
    long cleaningZ_steps = (long)(CLEANING_Z_INCH * STEPS_PER_INCH_XYZ);

    myServo.setAngle(35); // Set servo to cleaning angle
    moveToXYZ(cleaningX_steps, CLEANING_X_SPEED, cleaningY_steps, CLEANING_Y_SPEED, cleaningZ_steps, CLEANING_Z_SPEED);

    Serial.print("Starting short cleaning burst... (");
    Serial.print(runLabel);
    Serial.println(")");
    paintGun_ON();
    delay(500); // Keep paint gun on for 0.5 seconds
    paintGun_OFF();
    Serial.print("Cleaning burst complete. (");
    Serial.print(runLabel);
    Serial.println(")");
    // Note: Servo remains at cleaning angle, assuming painting functions will set their required angles. If not, add code here to return servo to a default angle.
    
    //! Move Z axis to home (0) before starting paint sequence
    Serial.print("Moving Z axis to home (0)... (");
    Serial.print(runLabel);
    Serial.println(")");
    // Use current X/Y positions from after the cleaning move
    moveToXYZ(stepperX->getCurrentPosition(), CLEANING_X_SPEED, stepperY_Left->getCurrentPosition(), CLEANING_Y_SPEED, 0, CLEANING_Z_SPEED);
    */
    // --- END: Comment out redundant cleaning burst in All_Sides ---
    // The PaintingState now handles a dedicated pre-paint clean using CleaningState.

    Serial.print("Z axis at 0 (assumed or handled by pre-clean). Starting painting. (");
    Serial.print(runLabel);
    Serial.println(")");
    // Note: Servo angle should be set by individual side patterns as needed.
    
    //! STEP 1: Paint right side
    Serial.print("Starting Right Side ("); Serial.print(runLabel); Serial.println(")");
    paintSide2Pattern();
    if (checkForHomeCommand()) {
        Serial.print("All Sides Painting ABORTED ("); Serial.print(runLabel); Serial.println(", after right side)");
        return false;
    }

    //! STEP 2: Paint back side
    Serial.print("Starting Back Side ("); Serial.print(runLabel); Serial.println(")");
    paintSide3Pattern();
    if (checkForHomeCommand()) {
        Serial.print("All Sides Painting ABORTED ("); Serial.print(runLabel); Serial.println(", after back side)");
        return false;
    }

    //! STEP 3: Paint front side (Commented Out)
    // Serial.print("Starting Front Side ("); Serial.print(runLabel); Serial.println(")");
    // paintSide1Pattern();
    // if (checkForHomeCommand()) {
    //     Serial.print("All Sides Painting ABORTED ("); Serial.print(runLabel); Serial.println(", after front side (Commented Out))");
    //     return false;
    // }
    
    //! STEP 4: Paint left side
    Serial.print("Starting Left Side ("); Serial.print(runLabel); Serial.println(")");
    paintSide4Pattern();
    if (checkForHomeCommand()) {
        Serial.print("All Sides Painting ABORTED ("); Serial.print(runLabel); Serial.println(", after left side)");
        return false;
    }
    
    Serial.print("All Sides Painting Sequence (");
    Serial.print(runLabel);
    Serial.println(") Completed.");
    return true;
} 

// Main function to be called externally
void paintAllSides() {
    Serial.println("Initiating Three-Run All Sides Painting Process.");

    // First run
    if (!_executeSinglePaintAllSidesSequence("Run 1")) {
        Serial.println("First run aborted. Full three-run process terminated.");
        return; // Abort if the first run was cancelled
    }

    Serial.println("First run of All Sides Painting finished.");
    Serial.println("Homing all axes before starting timer for second run...");

    Homing homingController1(engine, stepperX, stepperY_Left, stepperY_Right, stepperZ); // Renamed for clarity
    bool homingSuccess1 = homingController1.homeAllAxes(); 

    if (!homingSuccess1) {
        Serial.println("Homing after first run FAILED. Full three-run process terminated.");
        return; 
    }
    
    Serial.println("Homing successful (1). Starting X-axis loading bar movement for 30s.");

    const float LOADING_BAR_X_INCHES = 26.0f;
    long target_x_loading_bar_steps = (long)(LOADING_BAR_X_INCHES * STEPS_PER_INCH_XYZ); 
    float duration_seconds = ALL_SIDES_REPEAT_DELAY_MS / 1000.0f;

    if (duration_seconds < 0.1f || target_x_loading_bar_steps <= 0) { 
        Serial.println("Loading bar (1) fallback: Simple timed wait.");
        unsigned long simpleDelayStartTime = millis();
        while (millis() - simpleDelayStartTime < ALL_SIDES_REPEAT_DELAY_MS) {
            if (checkForHomeCommand()) {
                Serial.println("Home command during fallback wait (1). Process terminated.");
                return;
            }
            unsigned long time_elapsed_in_loop = millis() - simpleDelayStartTime;
            if (time_elapsed_in_loop >= ALL_SIDES_REPEAT_DELAY_MS) break;
            unsigned long time_remaining_in_loop = ALL_SIDES_REPEAT_DELAY_MS - time_elapsed_in_loop;
            delay(min(100UL, time_remaining_in_loop)); 
        }
    } else {
        float calculated_speed_hz = (float)target_x_loading_bar_steps / duration_seconds;
        unsigned int speed_to_set_hz = (unsigned int)max(1.0f, calculated_speed_hz); 
        
        Serial.printf("Loading Bar (1): Moving X over %.2f s.\n", duration_seconds);

        stepperX->setSpeedInHz(speed_to_set_hz);
        stepperX->setAcceleration(DEFAULT_X_ACCEL);
        stepperX->moveTo(target_x_loading_bar_steps);

        while (stepperX->isRunning()) {
            if (checkForHomeCommand()) {
                Serial.println("Home command during loading bar (1). Process terminated.");
                stepperX->forceStopAndNewPosition(stepperX->getCurrentPosition());
                return;
            }
            delay(1);
        }
        Serial.println("Loading bar movement (1) complete.");
    }

    // Second run
    Serial.println("Loading bar / delay (1) finished. Starting second run of All Sides Painting.");
    if (!_executeSinglePaintAllSidesSequence("Run 2")) {
        Serial.println("Second run aborted. Full three-run process terminated.");
        return;
    }
    Serial.println("Second run of All Sides Painting finished.");
    Serial.println("Homing all axes before starting timer for third run...");

    Homing homingController2(engine, stepperX, stepperY_Left, stepperY_Right, stepperZ); // Renamed
    bool homingSuccess2 = homingController2.homeAllAxes();

    if (!homingSuccess2) {
        Serial.println("Homing after second run FAILED. Full three-run process terminated.");
        return;
    }

    Serial.println("Homing successful (2). Starting X-axis loading bar movement for 30s.");
    // Re-use target_x_loading_bar_steps and duration_seconds as they are the same

    if (duration_seconds < 0.1f || target_x_loading_bar_steps <= 0) { 
        Serial.println("Loading bar (2) fallback: Simple timed wait.");
        unsigned long simpleDelayStartTime = millis();
        while (millis() - simpleDelayStartTime < ALL_SIDES_REPEAT_DELAY_MS) {
            if (checkForHomeCommand()) {
                Serial.println("Home command during fallback wait (2). Process terminated.");
                return;
            }
            unsigned long time_elapsed_in_loop = millis() - simpleDelayStartTime;
            if (time_elapsed_in_loop >= ALL_SIDES_REPEAT_DELAY_MS) break;
            unsigned long time_remaining_in_loop = ALL_SIDES_REPEAT_DELAY_MS - time_elapsed_in_loop;
            delay(min(100UL, time_remaining_in_loop)); 
        }
    } else {
        float calculated_speed_hz = (float)target_x_loading_bar_steps / duration_seconds;
        unsigned int speed_to_set_hz = (unsigned int)max(1.0f, calculated_speed_hz); 
        
        Serial.printf("Loading Bar (2): Moving X over %.2f s.\n", duration_seconds);

        stepperX->setSpeedInHz(speed_to_set_hz);
        stepperX->setAcceleration(DEFAULT_X_ACCEL);
        stepperX->moveTo(target_x_loading_bar_steps); // Assumes X is at 0 after homing

        while (stepperX->isRunning()) {
            if (checkForHomeCommand()) {
                Serial.println("Home command during loading bar (2). Process terminated.");
                stepperX->forceStopAndNewPosition(stepperX->getCurrentPosition());
                return;
            }
            delay(1);
        }
        Serial.println("Loading bar movement (2) complete.");
    }
    
    // Third run
    Serial.println("Loading bar / delay (2) finished. Starting third run of All Sides Painting.");
    if (!_executeSinglePaintAllSidesSequence("Run 3")) {
        Serial.println("Third run aborted. Full three-run process terminated.");
        return;
    }

    Serial.println("Three-Run All Sides Painting Process Fully Completed.");
} 