#include "states/HomingState.h"
#include <Arduino.h>
// #include <Bounce2.h> // No longer needed here
#include <FastAccelStepper.h>
#include "utils/settings.h"
#include "system/machine_state.h"
#include "system/StateMachine.h" 
// #include "motors/XYZ_Movements.h" // XYZ_Movements likely included via Homing.h if needed
#include "motors/Homing.h" // Include the new Homing class header

// // Declare global variables used by the homing state
// const unsigned long HOMING_SWITCH_DEBOUNCE_MS = 3; // Moved to Homing class
bool homeAfterMovement = false; // Keep this if it's used elsewhere for triggering homing

// // Bounce objects for debouncing the homing switches - Moved to Homing class
// Bounce xHomeSwitch = Bounce();
// Bounce yLeftHomeSwitch = Bounce();
// Bounce yRightHomeSwitch = Bounce();
// Bounce zHomeSwitch = Bounce();

// Need access to the global stepper pointers
extern FastAccelStepper *stepperX;
extern FastAccelStepper *stepperY_Left;
extern FastAccelStepper *stepperY_Right;
extern FastAccelStepper *stepperZ;
// extern FastAccelStepper *rotationStepper; // Declared in Rotation_Motor.h

// Machine state variables
// bool isHoming = false; // Moved to Homing class or managed internally
void setMachineState(int state);
void clearMachineState();

// Reference to the state machine
extern StateMachine* stateMachine;

// // Utility function to convert inches to steps - Moved to Homing class or shared location
// long inchesToStepsXYZ(float inches) {
//     return (long)(inches * STEPS_PER_INCH_XYZ);
// }

// // Servo control function - Forward declared/included via Homing.h if needed
// void setPitchServoAngle(int angle);

// // Import the servoInitialized variable from servo_control.cpp - Handled in Homing.cpp
// extern bool servoInitialized;

// Externally defined objects (likely in Setup.cpp)
extern FastAccelStepperEngine engine; 
// // Extern Bounce objects are not needed here anymore
// extern Bounce debounceX; 
// extern Bounce debounceY_Left;
// extern Bounce debounceY_Right;
// extern Bounce debounceZ;

HomingState::HomingState() {
    // Constructor implementation (can be empty if nothing needed)
}

void HomingState::enter() {
    Serial.println("Entering Homing State");
    
    // // Initialize bounce objects for homing - Moved to Homing class constructor
    // xHomeSwitch.attach(X_HOME_SWITCH);
    // xHomeSwitch.interval(HOMING_SWITCH_DEBOUNCE_MS);
    // ... etc ...
    
    //! Create Homing object and run homing sequence
    Homing homingController(engine, stepperX, stepperY_Left, stepperY_Right, stepperZ);
    bool homingSuccess = homingController.homeAllAxes();

    // After homing attempt, transition back to idle state (or handle error)
    // The Homing class now handles setting MACHINE_HOMING and MACHINE_IDLE/MACHINE_ERROR internally.
    // We just need to tell the StateMachine to transition.
    if (stateMachine) {
        if (homingSuccess) {
             Serial.println("Homing successful, transitioning to IDLE state.");
             // clearMachineState(); // Called within homeAllAxes on success
             stateMachine->changeState(stateMachine->getIdleState());
        } else {
             Serial.println("Homing failed, transitioning to IDLE state (or an error state if implemented)." );
             stateMachine->changeState(stateMachine->getIdleState());
        }
    } else {
         Serial.println("ERROR: StateMachine pointer is null after homing attempt!");
         // Handle error, maybe manual state setting?
         if (homingSuccess) {
             clearMachineState(); // Sets to IDLE
         } else {
             setMachineState(MachineState::ERROR); // Corrected call
         }
    }
}

void HomingState::update() {
    // Homing is blocking within the enter() method, so update() is currently unused for homing progress.
    // If homing were made non-blocking, progress checks would go here.
}

void HomingState::exit() {
     Serial.println("Exiting Homing State");
    // No longer need to manage isHoming flag here
    setMachineState(MachineState::UNKNOWN); // Or IDLE if appropriate after homing
}

const char* HomingState::getName() const {
    return "HOMING";
}

// // Implementation of the homing logic - MOVED TO Homing.cpp
// bool homeAllAxes() { 
//    // ... entire function removed ...
// } 