#include "states/ManualMoveState.h"
#include <Arduino.h>
#include <FastAccelStepper.h>
#include "motors/XYZ_Movements.h"
#include "motors/ServoMotor.h"
#include "utils/settings.h"
#include "system/StateMachine.h"

// External servo instance
extern ServoMotor myServo;

// External StateMachine instance (needed for checks)
extern StateMachine* stateMachine;

// Speed settings for manual movements (adjust as needed)
const unsigned int MANUAL_MOVE_X_SPEED = 10000; 
const unsigned int MANUAL_MOVE_Y_SPEED = 10000;
const unsigned int MANUAL_MOVE_Z_SPEED = 4000;

//* ************************************************************************
//* ************************ MANUAL MOVE STATE ***************************
//* ************************************************************************

ManualMoveState::ManualMoveState() : isMoving(false) {
    // Constructor
}

void ManualMoveState::enter() {
    Serial.println("Entering Manual Move State");
    isMoving = false;
}

void ManualMoveState::update() {
    // The update loop in manual mode primarily waits for external commands.
    // Movement logic is handled by moveToPosition triggered via WebSocket.
    if (isMoving) {
        // Optional: Could add logic here to monitor movement completion 
        // if moveToXYZ becomes non-blocking in the future.
        // For now, assuming moveToXYZ is blocking.
    }
}

void ManualMoveState::exit() {
    Serial.println("Exiting Manual Move State");
    isMoving = false;
}

const char* ManualMoveState::getName() const {
    return "MANUAL";
}

void ManualMoveState::moveToPosition(long targetX, long targetY, long targetZ, int targetAngle) {
    // Check if we are actually in the manual move state using StateMachine
    if (!stateMachine || stateMachine->getCurrentState() != this) { 
        Serial.println("Error: Cannot execute manual move outside of Manual Move State.");
        return;
    }

    Serial.print("Manual Move Command Received: X=");
    Serial.print(targetX); 
    Serial.print(", Y=");
    Serial.print(targetY);
    Serial.print(", Z=");
    Serial.print(targetZ);
    Serial.print(", Angle=");
    Serial.println(targetAngle);

    isMoving = true;

    //! Set Servo Angle
    myServo.setAngle(targetAngle);
    Serial.print("Servo set to ");
    Serial.print(targetAngle);
    Serial.println(" degrees");
    delay(500); // Allow servo time to move

    //! Move XYZ Axis
    // Convert steps back to inches/units if needed for logging or validation, 
    // but moveToXYZ expects steps.
    Serial.println("Moving XYZ axes...");
    moveToXYZ(targetX, MANUAL_MOVE_X_SPEED, targetY, MANUAL_MOVE_Y_SPEED, targetZ, MANUAL_MOVE_Z_SPEED);
    Serial.println("XYZ move complete.");

    isMoving = false;
} 