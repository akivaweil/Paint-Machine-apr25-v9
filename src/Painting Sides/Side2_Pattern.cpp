#include <Arduino.h>
#include "../../include/motors/XYZ_Movements.h"
#include "../../include/utils/settings.h"
#include "../../include/motors/Rotation_Motor.h"
#include "../../include/hardware/paintGun_Functions.h"
#include "../../include/hardware/pressurePot_Functions.h"
#include "../../include/settings/painting.h"
#include "../../include/persistence/PaintingSettings.h"
#include <FastAccelStepper.h>
#include "../../include/motors/ServoMotor.h"
#include "../../include/web/Web_Dashboard_Commands.h"

// External references to stepper motors
extern FastAccelStepper *stepperX;
extern FastAccelStepper *stepperY_Left;
extern FastAccelStepper *stepperZ;
extern ServoMotor myServo;
extern PaintingSettings paintingSettings;

//* ************************************************************************
//* ************************** SIDE 2 PAINTING ***************************
//* ************************************************************************
//* SIDE 2 PAINTING PATTERN (Vertical Sweeps)
//*
//*    P1 (SIDE2_START_X,Y)         P3                      P5                      P7                      P9
//*          ↓                     ↑                       ↓                       ↑                       ↓
//*    P2                      P4                      P6                      P8                      P10 (END)
//*          ←--------------------- ←--------------------- ←--------------------- ←--------------------- ← (Assumed X- shift)
//*
//* Sequence: P1→P2 (Y+), Shift X, P3→P4 (Y-), Shift X, P5→P6 (Y+), ... (Paint ON during vertical Y movements)
//*

void paintSide2Pattern() {
    Serial.println("Starting Side 2 Pattern Painting");

    int servoAngle = paintingSettings.getServoAngleSide2();

    //! Set Servo Angle FIRST
    myServo.setAngle(servoAngle);
    Serial.println("Servo set to: " + String(servoAngle) + " degrees for Side 2");

    //! STEP 0: Turn on pressure pot
    PressurePot_ON();

    //! STEP 1: Move to side 2 painting Z height
    long zPos = (long)(paintingSettings.getSide2ZHeight() * STEPS_PER_INCH_XYZ);
    long sideZPos = (long)(paintingSettings.getSide2SideZHeight() * STEPS_PER_INCH_XYZ);

    moveToXYZ(stepperX->getCurrentPosition(), DEFAULT_X_SPEED,
              stepperY_Left->getCurrentPosition(), DEFAULT_Y_SPEED,
              sideZPos, DEFAULT_Z_SPEED);

    //! STEP 2: Rotate to the side 2 position
    rotateToAngle(SIDE2_ROTATION_ANGLE);
    Serial.println("Rotated to Side 2 position");

    //! STEP 3: Move to start position (P1)
    long startX = (long)(paintingSettings.getSide4StartX() * STEPS_PER_INCH_XYZ); // Swapped from getSide2StartX
    long startY = (long)(paintingSettings.getSide4StartY() * STEPS_PER_INCH_XYZ); // Swapped from getSide2StartY
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    Serial.println("Moved to Side 4's designated start position (P1) for Side 2 pattern"); // Log updated

    //! STEP 4: Lower to painting Z height
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);

    //! STEP 5: Execute side 2 painting pattern
    long currentX = startX;
    long currentY = startY;
    long sweepYDistance = (long)(paintingSettings.getSide2SweepY() * STEPS_PER_INCH_XYZ);
    long shiftXDistance = (long)(paintingSettings.getSide2ShiftX() * STEPS_PER_INCH_XYZ);
    long paint_x_speed = paintingSettings.getSide2PaintingXSpeed();
    long paint_y_speed = paintingSettings.getSide2PaintingYSpeed();

    // First sweep: Y+ direction (P1 to P2)
    Serial.println("Side 2 Pattern: First sweep Y+");
    paintGun_ON();
    currentY += sweepYDistance;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting ABORTED due to home command");
        return;
    }

    // First shift: X direction (e.g., P2 to P3's X)
    Serial.println("Side 2 Pattern: Shift X");
    currentX += shiftXDistance; // Assuming Side 2 X shift is positive for this pattern structure
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);

    // Second sweep: Y- direction
    Serial.println("Side 2 Pattern: Second sweep Y-");
    paintGun_ON();
    currentY -= sweepYDistance;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting ABORTED due to home command");
        return;
    }

    // Second shift: X direction
    Serial.println("Side 2 Pattern: Shift X");
    currentX += shiftXDistance;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);

    // Third sweep: Y+ direction
    Serial.println("Side 2 Pattern: Third sweep Y+");
    paintGun_ON();
    currentY += sweepYDistance;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting ABORTED due to home command");
        return;
    }

    // Third shift: X direction
    Serial.println("Side 2 Pattern: Shift X");
    currentX += shiftXDistance;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);

    // Fourth sweep: Y- direction
    Serial.println("Side 2 Pattern: Fourth sweep Y-");
    paintGun_ON();
    currentY -= sweepYDistance;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting ABORTED due to home command");
        return;
    }

    // Fourth shift: X direction
    Serial.println("Side 2 Pattern: Shift X");
    currentX += shiftXDistance;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);

    // Fifth sweep: Y+ direction
    Serial.println("Side 2 Pattern: Fifth sweep Y+");
    paintGun_ON();
    currentY += sweepYDistance;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting ABORTED due to home command");
        return;
    }

    //! STEP 8: Raise to safe Z height
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);

    Serial.println("Side 2 Pattern Painting Completed");
}