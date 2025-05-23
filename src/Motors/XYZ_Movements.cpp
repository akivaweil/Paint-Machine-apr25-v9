#include "motors/XYZ_Movements.h"
#include <Arduino.h>
#include "utils/settings.h" // Likely needed for pin definitions, steps/mm, etc.

// Include motor control library
#include <FastAccelStepper.h>
#include <Bounce2.h>   // For debouncing limit switches

// Define stepper engine and steppers (example)
extern FastAccelStepperEngine engine; // Use the global one from Setup.cpp
extern FastAccelStepper *stepperX;
extern FastAccelStepper *stepperY_Left; // Renamed from stepperY
extern FastAccelStepper *stepperY_Right; // Added second Y motor
extern FastAccelStepper *stepperZ;

// Switch debouncing objects
extern Bounce debounceX;
extern Bounce debounceY_Left; // Renamed from debounceY
extern Bounce debounceY_Right; // Added second Y debouncer
extern Bounce debounceZ;

//* ************************************************************************
//* ************************* XYZ MOVEMENTS **************************
//* ************************************************************************

/* REMOVED - Logic moved to Setup.cpp
void setupMotors() {
    Serial.println("Setting up Motors...");
    engine.init();
    
    // Example Setup - Replace with actual pins and settings from settings.h
    // stepperX = engine.stepperConnectToPins(X_STEP_PIN, X_DIR_PIN);
    // if (stepperX) {
    //     stepperX->setEnablePin(X_ENABLE_PIN);
    //     stepperX->setAutoEnable(true);
    // }

    // Setup Y and Z similarly...
    
    // Setup limit switches - moved from setupFunctionality()
    pinMode(X_HOME_SWITCH, INPUT_PULLUP);
    pinMode(Y_LEFT_HOME_SWITCH, INPUT_PULLUP);
    pinMode(Y_RIGHT_HOME_SWITCH, INPUT_PULLUP);
    pinMode(Z_HOME_SWITCH, INPUT_PULLUP);
    
    // Setup Bounce2 for debouncing
    debounceX.attach(X_HOME_SWITCH);
    debounceX.interval(DEBOUNCE_INTERVAL);
    
    debounceY_Left.attach(Y_LEFT_HOME_SWITCH);
    debounceY_Left.interval(DEBOUNCE_INTERVAL);
    
    debounceY_Right.attach(Y_RIGHT_HOME_SWITCH);
    debounceY_Right.interval(DEBOUNCE_INTERVAL);

    debounceZ.attach(Z_HOME_SWITCH);
    debounceZ.interval(DEBOUNCE_INTERVAL);

    Serial.println("Motors and Switches Setup Complete.");
}
*/

void moveToXYZ(long x, unsigned int xSpeed, long y, unsigned int ySpeed, long z, unsigned int zSpeed) {
    // Set speed for each stepper individually
    stepperX->setSpeedInHz(xSpeed);
    stepperY_Left->setSpeedInHz(ySpeed); // Renamed
    stepperY_Right->setSpeedInHz(ySpeed); // Added second Y motor speed
    stepperZ->setSpeedInHz(zSpeed);
    
    // Command the move to the absolute position
    stepperX->moveTo(x);
    stepperY_Left->moveTo(y); // Renamed
    stepperY_Right->moveTo(y); // Added second Y motor move
    stepperZ->moveTo(z);
    
    // Wait until all steppers have completed their movements
    while (stepperX->isRunning() || stepperY_Left->isRunning() || stepperY_Right->isRunning() || stepperZ->isRunning()) { // Updated condition
        // Check for limit switches while running
        checkMotors();
        delay(10); // Small delay to prevent CPU hogging
    }
    
    Serial.printf("Move complete - Position: X:%ld Y_L:%ld Y_R:%ld Z:%ld\n", stepperX->getCurrentPosition(), stepperY_Left->getCurrentPosition(), stepperY_Right->getCurrentPosition(), stepperZ->getCurrentPosition()); // Updated printf
}

// This function replaces checkSwitches from Functionality.cpp
void checkMotors() {
    // Update debouncers
    debounceX.update();
    debounceY_Left.update(); // Renamed
    debounceY_Right.update(); // Added second Y debouncer update
    debounceZ.update();

    // Read switch states
    bool limitX = debounceX.read() == HIGH; // HIGH when triggered (assuming active high)
    bool limitY_Left = debounceY_Left.read() == HIGH; // Renamed & active high
    bool limitY_Right = debounceY_Right.read() == HIGH; // Added second Y switch read & active high
    bool limitZ = debounceZ.read() == HIGH; // active high
    
    // Example of using switch readings (add your own logic)
    if (limitX) {
        Serial.println("X limit switch triggered");
        // Take action like stopping X motor
        // stepperX->forceStop(); // Example action
    }
    
    // Similar handling for Y and Z
    if (limitY_Left) {
        Serial.println("Y Left limit switch triggered");
        // Take action like stopping Y motors during homing or if unexpected
        // stepperY_Left->forceStop(); // Example action
        // stepperY_Right->forceStop(); // Example action (if gantry safety requires stopping both)
    }
    if (limitY_Right) {
        Serial.println("Y Right limit switch triggered");
        // Take action like stopping Y motors during homing or if unexpected
        // stepperY_Left->forceStop(); // Example action (if gantry safety requires stopping both)
        // stepperY_Right->forceStop(); // Example action
    }
    if (limitZ) {
        Serial.println("Z limit switch triggered");
        // Take action like stopping Z motor
        // stepperZ->forceStop(); // Example action
    }
}

