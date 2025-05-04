#include <Arduino.h>
#include "../../include/motors/XYZ_Movements.h"       // For moveToXYZ
#include "../../include/utils/settings.h"         // For STEPS_PER_INCH, default speeds
#include "../../include/motors/Rotation_Motor.h"     // For rotateToAngle
#include "../../include/hardware/paintGun_Functions.h" // For paintGun_ON/OFF
#include "../../include/hardware/pressurePot_Functions.h" // For PressurePot_ON
#include <FastAccelStepper.h>
#include "../../include/persistence/PaintingSettings.h" // Include for accessing saved settings
#include "../../include/settings/painting.h"         // For painting-specific constants (SIDE1_Z_HEIGHT etc.)
#include "../../include/persistence/persistence.h"     // For loading servo angle
#include "../../include/motors/ServoMotor.h"         // For ServoMotor class
#include "states/PaintingState.h" // Correct filename
#include "settings/pins.h"        // Keep this one
#include "../../include/web/Web_Dashboard_Commands.h" // For checkForHomeCommand

// External references to stepper motors
extern FastAccelStepper *stepperX;
extern FastAccelStepper *stepperY_Left;
extern FastAccelStepper *stepperZ;
// extern PaintingSettings paintingSettings; // Might not be needed directly here
extern ServoMotor myServo; // Declare external servo instance

//* ************************************************************************
//* *************************** SIDE 1 *************************************
//* ************************************************************************

void paintSide1Pattern() {
    Serial.println("Starting Side 1 Pattern Painting");

    //! Load Servo Angle
    // persistence.begin(); // REMOVED
    int servoAngle = persistence.loadInt(SERVO_ANGLE_SIDE1_KEY, 90);
    // persistence.end(); // REMOVED

    //! Set Servo Angle FIRST
    myServo.setAngle(servoAngle);
    Serial.println("Servo set to: " + String(servoAngle) + " degrees for Side 1 side");

    //! STEP 0: Turn on pressure pot
    PressurePot_ON();

    //! STEP 1: Move to side 1 painting Z height
    // Use constants directly from settings/painting.h
    long zPos = (long)(paintingSettings.getSide1ZHeight() * STEPS_PER_INCH_XYZ); // Use getter
    long sideZPos = (long)(paintingSettings.getSide1SideZHeight() * STEPS_PER_INCH_XYZ); // Use getter

    // Use constants from utils/settings.h for default speeds
    moveToXYZ(stepperX->getCurrentPosition(), DEFAULT_X_SPEED,
              stepperY_Left->getCurrentPosition(), DEFAULT_Y_SPEED,
              sideZPos, DEFAULT_Z_SPEED);

    //! STEP 2: Rotate to the side 1 position
    rotateToAngle(SIDE1_ROTATION_ANGLE); // Speed likely handled within rotateToAngle
    Serial.println("Rotated to side 1 position");

    //! STEP 3: Move to start position (P2)
    long startX = (long)(paintingSettings.getSide1StartX() * STEPS_PER_INCH_XYZ); // Use getter
    long startY = (long)(paintingSettings.getSide1StartY() * STEPS_PER_INCH_XYZ); // Use getter
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    Serial.println("Moved to side 1 pattern start position (P2)");

    //! STEP 4: Lower to painting Z height
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);

    //! STEP 5: Execute side 1 painting pattern
    long currentX = startX;
    long currentY = startY;
    long sweepYDistance = (long)(paintingSettings.getSide1SweepY() * STEPS_PER_INCH_XYZ); // Use getter
    long shiftXDistance = (long)(paintingSettings.getSide1ShiftX() * STEPS_PER_INCH_XYZ); // Use getter

    paintGun_ON();
    currentX += shiftXDistance;
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // // Use painting-specific speeds from settings/painting.h
    // // First sweep: Y+ direction (P2 to P1)
    // Serial.println("Side 1 Pattern: First sweep Y+ (P2 to P1)");
    // paintGun_ON();
    // currentY += sweepYDistance;
    // moveToXYZ(currentX, paintingSettings.getSide1PaintingXSpeed(), currentY, paintingSettings.getSide1PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed
    // paintGun_OFF(); // Turn off gun after sweep

    // // Check for home command after sweep
    // if (checkForHomeCommand()) {
    //     // Raise to safe Z height before aborting
    //     moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    //     Serial.println("Side 1 Pattern Painting ABORTED due to home command");
    //     return;
    // }

    // // First shift: X+ direction (P1 to P3)
    // Serial.println("Side 1 Pattern: Shift X+ (P1 to P3)");
    // currentX += shiftXDistance;
    // moveToXYZ(currentX, paintingSettings.getSide1PaintingXSpeed(), currentY, paintingSettings.getSide1PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed

    // // Second sweep: Y- direction (P3 to P4)
    // Serial.println("Side 1 Pattern: Second sweep Y- (P3 to P4)");
    // paintGun_ON();  // Turn on gun for sweep
    // currentY -= sweepYDistance;
    // moveToXYZ(currentX, paintingSettings.getSide1PaintingXSpeed(), currentY, paintingSettings.getSide1PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed
    // paintGun_OFF(); // Turn off gun after sweep

    // // Check for home command after sweep
    // if (checkForHomeCommand()) {
    //     // Raise to safe Z height before aborting
    //     moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    //     Serial.println("Side 1 Pattern Painting ABORTED due to home command");
    //     return;
    // }

    // // Second shift: X+ direction (P4 to P5)
    // Serial.println("Side 1 Pattern: Shift X+ (P4 to P5)");
    // currentX += shiftXDistance;
    // moveToXYZ(currentX, paintingSettings.getSide1PaintingXSpeed(), currentY, paintingSettings.getSide1PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed

    // // Third sweep: Y+ direction (P5 to P6)
    // Serial.println("Side 1 Pattern: Third sweep Y+ (P5 to P6)");
    // paintGun_ON();  // Turn on gun for sweep
    // currentY += sweepYDistance;
    // moveToXYZ(currentX, paintingSettings.getSide1PaintingXSpeed(), currentY, paintingSettings.getSide1PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed
    // paintGun_OFF(); // Turn off gun after sweep

    // // Check for home command after sweep
    // if (checkForHomeCommand()) {
    //     // Raise to safe Z height before aborting
    //     moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    //     Serial.println("Side 1 Pattern Painting ABORTED due to home command");
    //     return;
    // }

    // // Third shift: X+ direction (P6 to P7)
    // Serial.println("Side 1 Pattern: Shift X+ (P6 to P7)");
    // currentX += shiftXDistance;
    // moveToXYZ(currentX, paintingSettings.getSide1PaintingXSpeed(), currentY, paintingSettings.getSide1PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed

    // // Fourth sweep: Y- direction (P7 to P8)
    // Serial.println("Side 1 Pattern: Fourth sweep Y- (P7 to P8)");
    // paintGun_ON();  // Turn on gun for sweep
    // currentY -= sweepYDistance;
    // moveToXYZ(currentX, paintingSettings.getSide1PaintingXSpeed(), currentY, paintingSettings.getSide1PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed
    // paintGun_OFF(); // Turn off gun after sweep

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 1 Pattern Painting ABORTED due to home command");
        return;
    }

    //! STEP 8: Raise to safe Z height
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);

    Serial.println("Side 1 Pattern Painting Completed");
}
