#include "motors/Homing.h"
#include "motors/XYZ_Movements.h"
#include <Arduino.h>
#include <Bounce2.h>
#include <FastAccelStepper.h>
#include "utils/settings.h"
#include "system/machine_state.h" // Include the updated header

// Need access to the global rotation stepper pointer if used
// This is already included via Rotation_Motor.h in Homing.h
// extern FastAccelStepper *rotationStepper;

Homing::Homing(FastAccelStepperEngine& engine,
               FastAccelStepper* stepperX,
               FastAccelStepper* stepperY_Left,
               FastAccelStepper* stepperY_Right,
               FastAccelStepper* stepperZ)
    : _engine(engine),
      _stepperX(stepperX),
      _stepperY_Left(stepperY_Left),
      _stepperY_Right(stepperY_Right),
      _stepperZ(stepperZ)
{
    // Initialize bounce objects here
    _xHomeSwitch.attach(X_HOME_SWITCH);
    _xHomeSwitch.interval(HOMING_SWITCH_DEBOUNCE_MS);
    
    _yLeftHomeSwitch.attach(Y_LEFT_HOME_SWITCH);
    _yLeftHomeSwitch.interval(HOMING_SWITCH_DEBOUNCE_MS);
    
    _yRightHomeSwitch.attach(Y_RIGHT_HOME_SWITCH);
    _yRightHomeSwitch.interval(HOMING_SWITCH_DEBOUNCE_MS);
    
    _zHomeSwitch.attach(Z_HOME_SWITCH);
    _zHomeSwitch.interval(HOMING_SWITCH_DEBOUNCE_MS);
}

// Utility function implementation (keep it private to this class for now)
long Homing::inchesToStepsXYZ(float inches) {
    return (long)(inches * STEPS_PER_INCH_XYZ);
}

// Implementation of the homing logic, now as a class method
bool Homing::homeAllAxes() {
    Serial.println("Starting Home All Axes sequence...");
    // setMachineState(MachineState::HOMING); // REMOVED
    
    //! STEP 1: Initialize homing sequence
    Serial.println("Starting homing sequence...");

    //! STEP 2: Configure switch pins (pins attached in constructor)
    
    //! STEP 3: Set speeds and accelerations for homing movement
    _stepperX->setSpeedInHz(HOMING_SPEED_X);
    _stepperX->setAcceleration(HOMING_ACCEL_X);
    _stepperY_Left->setSpeedInHz(HOMING_SPEED_Y);
    _stepperY_Left->setAcceleration(HOMING_ACCEL_Y);
    _stepperY_Right->setSpeedInHz(HOMING_SPEED_Y);
    _stepperY_Right->setAcceleration(HOMING_ACCEL_Y);
    _stepperZ->setSpeedInHz(HOMING_SPEED_Z);
    _stepperZ->setAcceleration(HOMING_ACCEL_Z);
    
    //? Set rotation motor speeds (if it exists)
    if (rotationStepper) {
        rotationStepper->setSpeedInHz(DEFAULT_ROT_SPEED / 2); //? Half speed for homing
        rotationStepper->setAcceleration(DEFAULT_ROT_ACCEL / 2); //? Half acceleration for homing
    }
    
    //! STEP 4: Start all motors moving toward home switches
    Serial.println("Moving all axes toward home switches...");
    // Debug: Check direction pin state BEFORE runBackward()
    pinMode(X_DIR_PIN, OUTPUT); // Ensure pin mode is set if not already
    Serial.printf("  DEBUG: X_DIR_PIN (%d) state before runBackward: %d\n", X_DIR_PIN, digitalRead(X_DIR_PIN));
    _stepperX->runBackward(); 
    Serial.printf("  DEBUG: X_DIR_PIN (%d) state AFTER runBackward: %d\n", X_DIR_PIN, digitalRead(X_DIR_PIN));
    _stepperY_Left->runBackward();
    _stepperY_Right->runBackward();
    _stepperZ->runForward(); //? Z moves forward (UP) to home
    
    //? Start rotation homing (if it exists)
    if (rotationStepper) {
        rotationStepper->moveTo(0); //? Move to 0 position
    }
    
    //! STEP 5: Track homing status for each motor
    bool xHomed = false;
    bool yLeftHomed = false;
    bool yRightHomed = false;  // Add tracking for Y right
    bool zHomed = false;
    bool rotationHomed = (rotationStepper == NULL); //? True if no rotation stepper
    
    unsigned long startTime = millis();
    
    //! STEP 6: Monitor all switches simultaneously using Bounce2
    while (!xHomed || !yLeftHomed || !yRightHomed || !zHomed || !rotationHomed) {
        //? Check timeout
        if (millis() - startTime > HOMING_TIMEOUT_MS) {
            Serial.println("ERROR: Homing timeout!");
            //? Stop any motors that haven't homed yet
            if (!xHomed) _stepperX->forceStopAndNewPosition(_stepperX->getCurrentPosition());
            if (!yLeftHomed) _stepperY_Left->forceStopAndNewPosition(_stepperY_Left->getCurrentPosition());
            if (!yRightHomed) _stepperY_Right->forceStopAndNewPosition(_stepperY_Right->getCurrentPosition()); // Stop Y Right too
            if (!zHomed) _stepperZ->forceStopAndNewPosition(_stepperZ->getCurrentPosition());
            if (!rotationHomed && rotationStepper) rotationStepper->forceStopAndNewPosition(rotationStepper->getCurrentPosition());
            // setMachineState(MachineState::ERROR); // REMOVED - StateMachine handles transition
            return false;
        }
        
        //! Process X switch with Bounce2
        if (!xHomed) {
            _xHomeSwitch.update();
            if (_xHomeSwitch.read() == HIGH) { 
                _stepperX->forceStopAndNewPosition(0);
                xHomed = true;
                Serial.println("X Home switch triggered.");
            }
        }
        
        //! Process Y Left switch with Bounce2
        if (!yLeftHomed) {
            _yLeftHomeSwitch.update();
            if (_yLeftHomeSwitch.read() == HIGH) { 
                _stepperY_Left->forceStopAndNewPosition(0);
                yLeftHomed = true;
                Serial.println("Y Left Home switch triggered.");
            }
        }
        
        //! Process Y Right switch with Bounce2
        if (!yRightHomed) {
            _yRightHomeSwitch.update();
            if (_yRightHomeSwitch.read() == HIGH) { 
                _stepperY_Right->forceStopAndNewPosition(0);
                yRightHomed = true;
                Serial.println("Y Right Home switch triggered.");
            }
        }
        
        //! Process Z switch with Bounce2
        if (!zHomed) {
            _zHomeSwitch.update();
            if (_zHomeSwitch.read() == HIGH) { 
                _stepperZ->forceStopAndNewPosition(0);
                zHomed = true;
                Serial.println("Z Home switch triggered.");
            }
        }
        
        //? Check if rotation is complete
        if (!rotationHomed && rotationStepper && !rotationStepper->isRunning()) {
            rotationStepper->setCurrentPosition(0);
            rotationHomed = true;
            Serial.println("Rotation axis homed successfully.");
        }
        
        yield(); // Allow other tasks to run
    }
    
    //! STEP 7: All switches triggered
    Serial.println("All home switches triggered and rotation homed.");
    delay(5); //? Ensure motors stopped and positions registered
    
    //! STEP 8: Move away from switches simultaneously
    Serial.println("Moving all axes away from home switches...");
    long moveAwaySteps = inchesToStepsXYZ(HOMING_MOVE_AWAY_INCHES);
    
    //? Set slower accelerations for move-away phase
    _stepperX->setAcceleration(HOMING_MOVE_AWAY_ACCEL_X);
    _stepperY_Left->setAcceleration(HOMING_MOVE_AWAY_ACCEL_Y);
    _stepperY_Right->setAcceleration(HOMING_MOVE_AWAY_ACCEL_Y);
    _stepperZ->setAcceleration(HOMING_MOVE_AWAY_ACCEL_Z);
    
    // Debug: Check direction pin state BEFORE moveTo positive
    Serial.printf("  DEBUG: X_DIR_PIN (%d) state before moveTo(%ld): %d\n", X_DIR_PIN, moveAwaySteps, digitalRead(X_DIR_PIN));
    _stepperX->moveTo(moveAwaySteps, false); //? Non-blocking start
    Serial.printf("  DEBUG: X_DIR_PIN (%d) state AFTER moveTo(%ld): %d\n", X_DIR_PIN, moveAwaySteps, digitalRead(X_DIR_PIN));
    _stepperY_Left->moveTo(moveAwaySteps, false);
    _stepperY_Right->moveTo(moveAwaySteps, false);
    _stepperZ->moveTo(-moveAwaySteps, false); //? Z moves DOWN (negative) to move away
    
    //! STEP 9: Wait for all motors to complete the move away
    startTime = millis(); // Reset timer for move away
    unsigned long lastPrintTime = 0; // Debug print timer
    while (_stepperX->isRunning() || 
           _stepperY_Left->isRunning() || 
           _stepperY_Right->isRunning() || 
           _stepperZ->isRunning()) {
        if (millis() - startTime > 5000) { //? 5 second timeout for move away
            Serial.println("ERROR: Timeout moving away from switches!");
            _stepperX->forceStopAndNewPosition(_stepperX->getCurrentPosition());
            _stepperY_Left->forceStopAndNewPosition(_stepperY_Left->getCurrentPosition());
            _stepperY_Right->forceStopAndNewPosition(_stepperY_Right->getCurrentPosition());
            _stepperZ->forceStopAndNewPosition(_stepperZ->getCurrentPosition());
            // setMachineState(MachineState::ERROR); // REMOVED - StateMachine handles transition
            return false;
        }
        
        //! Debug prints every 250ms
        if (millis() - lastPrintTime > 250) {
            Serial.printf("  MoveAway Status: X_run=%d (pos:%ld), YL_run=%d (pos:%ld), YR_run=%d (pos:%ld), Z_run=%d (pos:%ld)\n",
                          _stepperX->isRunning(), _stepperX->getCurrentPosition(),
                          _stepperY_Left->isRunning(), _stepperY_Left->getCurrentPosition(),
                          _stepperY_Right->isRunning(), _stepperY_Right->getCurrentPosition(),
                          _stepperZ->isRunning(), _stepperZ->getCurrentPosition());
            lastPrintTime = millis();
        }
        
        yield(); // Allow other tasks to run
    }
    
    //! STEP 10: Set final logical position to 0 for all axes
    Serial.println("Setting logical positions to 0.");
    _stepperX->setCurrentPosition(0);
    _stepperY_Left->setCurrentPosition(0);
    _stepperY_Right->setCurrentPosition(0);
    _stepperZ->setCurrentPosition(0);
    //? Rotation already set to 0 earlier
    
    //! STEP 11: Homing completed successfully
    Serial.println("Homing sequence completed successfully.");
    
    //! Restore Default Accelerations
    Serial.println("Restoring default accelerations...");
    _stepperX->setAcceleration(DEFAULT_X_ACCEL);
    _stepperY_Left->setAcceleration(DEFAULT_Y_ACCEL);
    _stepperY_Right->setAcceleration(DEFAULT_Y_ACCEL);
    _stepperZ->setAcceleration(DEFAULT_Z_ACCEL);
    if (rotationStepper) {
        rotationStepper->setAcceleration(DEFAULT_ROT_ACCEL); // Restore rotation accel too
    }

    bool allHomed = xHomed && yLeftHomed && yRightHomed && zHomed;

    if (allHomed) {
        Serial.println("All axes homed successfully.");
        // clearMachineState(); // REMOVED - StateMachine handles transition
    } else {
        Serial.println("Homing failed for one or more axes.");
        // setMachineState(MachineState::ERROR); // REMOVED - StateMachine handles transition/error reporting
    }
    return allHomed;
}

// REMOVED individual homing functions like homeZ() as they were placeholders/not declared in Homing.h
// If needed, they should be declared in the header and implemented properly.
/*
bool Homing::homeZ() {
    // ... implementation ...
}
*/

// REMOVED Homing::exit() as it doesn't belong to this class
/*
void Homing::exit() {
     Serial.println("Exiting Homing State");
}
*/

// Placeholder or ensure this is defined elsewhere and included correctly
// If XYZ_Movements.cpp defines this, ensure it's declared in XYZ_Movements.h
// void setPitchServoAngle(int angle) {
//     // Implementation
// } 