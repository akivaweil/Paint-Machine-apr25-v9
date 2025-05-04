#include "states/ManualMoveState.h"
#include <Arduino.h>
#include <FastAccelStepper.h>
#include "system/machine_state.h"
#include "motors/XYZ_Movements.h"
#include "motors/ServoMotor.h"
#include "utils/settings.h"

// External servo instance
extern ServoMotor myServo;

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
    setMachineState(MACHINE_MANUAL_MOVE);
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
    clearMachineState();
    isMoving = false;
}

void ManualMoveState::moveToPosition(long targetX, long targetY, long targetZ, int targetAngle) {
    if (getMachineState() != MACHINE_MANUAL_MOVE) {
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