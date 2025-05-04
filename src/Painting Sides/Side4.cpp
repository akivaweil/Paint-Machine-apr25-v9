#include <Arduino.h>
#include "../../include/motors/XYZ_Movements.h"
#include "../../include/utils/settings.h"
#include "../../include/motors/Rotation_Motor.h"
#include "../../include/hardware/paintGun_Functions.h"
#include "../../include/hardware/pressurePot_Functions.h"
#include "../../include/settings/painting.h"         // For painting-specific constants (SIDE4_Z_HEIGHT etc.)
#include "../../include/persistence/PaintingSettings.h" // Include for accessing saved settings
#include <FastAccelStepper.h>
// #include "../../include/persistence/PaintingSettings.h" // Not directly needed?
#include "../../include/persistence/persistence.h"     // For loading servo angle
#include "../../include/motors/ServoMotor.h"         // For ServoMotor class
#include "../../include/web/Web_Dashboard_Commands.h" // For checkForHomeCommand

// External references to stepper motors
extern FastAccelStepper *stepperX;
extern FastAccelStepper *stepperY_Left;
extern FastAccelStepper *stepperZ;
// extern PaintingSettings paintingSettings; // Use constants from painting.h instead
extern ServoMotor myServo;

//* ************************************************************************
//* ************************** SIDE 4 PAINTING ***************************
//* ************************************************************************

void paintSide4Pattern() {
    Serial.println("Starting Side 4 Pattern Painting");

    //! Load Servo Angle
    // persistence.begin(); // REMOVED
    int servoAngle = persistence.loadInt(SERVO_ANGLE_SIDE4_KEY, 90);
    // persistence.end(); // REMOVED

    //! Set Servo Angle FIRST
    myServo.setAngle(servoAngle);
    Serial.println("Servo set to: " + String(servoAngle) + " degrees for Side 4 side");

    //! STEP 0: Turn on pressure pot
    PressurePot_ON();

    //! STEP 1: Move to side 4 painting Z height
    long zPos = (long)(paintingSettings.getSide4ZHeight() * STEPS_PER_INCH_XYZ); // Use getter
    long sideZPos = (long)(paintingSettings.getSide4SideZHeight() * STEPS_PER_INCH_XYZ); // Use getter

    moveToXYZ(stepperX->getCurrentPosition(), DEFAULT_X_SPEED,
              stepperY_Left->getCurrentPosition(), DEFAULT_Y_SPEED,
              sideZPos, DEFAULT_Z_SPEED);

    //! STEP 2: Rotate to the side 4 position
    rotateToAngle(SIDE4_ROTATION_ANGLE);
    Serial.println("Rotated to side 4 position");

    //! STEP 3: Move to start position (P1)
    long startX = (long)(paintingSettings.getSide4StartX() * STEPS_PER_INCH_XYZ); // Use getter
    long startY = (long)(paintingSettings.getSide4StartY() * STEPS_PER_INCH_XYZ); // Use getter
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    Serial.println("Moved to side 4 pattern start position (P1)");

    //! STEP 4: Lower to painting Z height
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);

    //! STEP 5: Execute side 4 painting pattern
    long currentX = startX;
    long currentY = startY;
    long sweepYDistance = (long)(paintingSettings.getSide4SweepY() * STEPS_PER_INCH_XYZ); // Use getter
    long shiftXDistance = (long)(paintingSettings.getSide4ShiftX() * STEPS_PER_INCH_XYZ); // Use getter

    // First sweep: Y+ direction (P2 to P1)
    Serial.println("Side 4 Pattern: First sweep Y+ (P2 to P1)");
    paintGun_ON();
    currentY += sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting ABORTED due to home command");
        return;
    }

    // First shift: X+ direction (P1 to P3)
    Serial.println("Side 4 Pattern: Shift X+ (P1 to P3)");
    currentX += shiftXDistance;
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed

    // Second sweep: Y- direction (P3 to P4)
    Serial.println("Side 4 Pattern: Second sweep Y- (P3 to P4)");
    paintGun_ON();
    currentY -= sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting ABORTED due to home command");
        return;
    }

    // Second shift: X+ direction (P4 to P5)
    Serial.println("Side 4 Pattern: Shift X+ (P4 to P5)");
    currentX += shiftXDistance;
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed

    // Third sweep: Y+ direction (P5 to P6)
    Serial.println("Side 4 Pattern: Third sweep Y+ (P5 to P6)");
    paintGun_ON();
    currentY += sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting ABORTED due to home command");
        return;
    }

    // Third shift: X+ direction (P6 to P7)
    Serial.println("Side 4 Pattern: Shift X+ (P6 to P7)");
    currentX += shiftXDistance;
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed

    // Fourth sweep: Y- direction (P7 to P8)
    Serial.println("Side 4 Pattern: Fourth sweep Y- (P7 to P8)");
    paintGun_ON();
    currentY -= sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting ABORTED due to home command");
        return;
    }

    // Fourth shift: X+ direction (P8 to P9)
    Serial.println("Side 4 Pattern: Shift X+ (P8 to P9)");
    currentX += shiftXDistance;
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed

    // Fifth sweep: Y+ direction (P9 to P10)
    Serial.println("Side 4 Pattern: Fifth sweep Y+ (P9 to P10)");
    paintGun_ON();
    currentY += sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // Use getters for speed
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting ABORTED due to home command");
        return;
    }

    //! STEP 8: Raise to safe Z height
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);

    Serial.println("Side 4 Pattern Painting Completed");
}
