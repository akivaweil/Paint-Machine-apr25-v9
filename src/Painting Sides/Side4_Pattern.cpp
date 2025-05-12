#include <Arduino.h>
#include "../../include/motors/XYZ_Movements.h"
#include "../../include/utils/settings.h"
#include "../../include/motors/Rotation_Motor.h"
#include "../../include/hardware/paintGun_Functions.h"
#include "../../include/hardware/pressurePot_Functions.h"
#include <FastAccelStepper.h>
#include "../../include/settings/painting.h"
#include "../../include/motors/ServoMotor.h"
#include "../../include/persistence/PaintingSettings.h"
#include "../../include/web/Web_Dashboard_Commands.h"

// External references to stepper motors
extern FastAccelStepper *stepperX;
extern FastAccelStepper *stepperY_Left;
extern FastAccelStepper *stepperZ;
extern ServoMotor myServo;
extern PaintingSettings paintingSettings;

//* ************************************************************************
//* ************************** SIDE 4 PAINTING ***************************
//* ************************************************************************
//* SIDE 4 PAINTING PATTERN (Vertical Sweeps, specific sequence)
//*
//*    P9                      P7                      P5                      P3                      P1 (SIDE4_START_X,Y)
//*          ↓                     ↑                       ↓                       ↑                       ↓
//*    P10 (END)               P8                      P6                      P4                      P2
//*          ←--------------------- ←--------------------- ←--------------------- ←--------------------- ← (Assumed X- shift)
//*
//* Sequence based on original logic provided: Move to a calculated start (derived from P7's equivalent for Side 4),
//* then Y- sweep, X shift, Y+ sweep.

void paintSide4Pattern() {
    Serial.println("Starting Side 4 Pattern Painting");

    int servoAngle = paintingSettings.getServoAngleSide4();

    //! Set Servo Angle FIRST
    myServo.setAngle(servoAngle);
    Serial.println("Servo set to: " + String(servoAngle) + " degrees for Side 4");

    //! STEP 0: Turn on pressure pot
    PressurePot_ON();

    //! STEP 1: Move to Side 4 painting Z height
    long zPos = (long)(paintingSettings.getSide4ZHeight() * STEPS_PER_INCH_XYZ);
    long sideZPos = (long)(paintingSettings.getSide4SideZHeight() * STEPS_PER_INCH_XYZ);

    moveToXYZ(stepperX->getCurrentPosition(), DEFAULT_X_SPEED,
              stepperY_Left->getCurrentPosition(), DEFAULT_Y_SPEED,
              sideZPos, DEFAULT_Z_SPEED);

    //! STEP 2: Rotate to the Side 4 position
    rotateToAngle(SIDE4_ROTATION_ANGLE);
    Serial.println("Rotated to Side 4 position");

    //! STEP 3: Move to start position for Side 4 pattern logic
    long startX_steps = (long)(paintingSettings.getSide2StartX() * STEPS_PER_INCH_XYZ); // Swapped from getSide4StartX
    long startY_steps = (long)(paintingSettings.getSide2StartY() * STEPS_PER_INCH_XYZ); // Swapped from getSide4StartY
    // The original logic for this pattern started at a calculated offset point.
    // We will use Side 2's startX/Y (now assigned to Side 4's use) as the base for these calculations.
    moveToXYZ(startX_steps, DEFAULT_X_SPEED, startY_steps, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    Serial.println("Moved to Side 2's designated start position. Actual painting starts after lowering and potential initial move for Side 4 pattern."); // Log updated

    //! STEP 4: Lower to painting Z height
    moveToXYZ(startX_steps, DEFAULT_X_SPEED, startY_steps, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);

    // -- START OF SIDE 4 PATTERN --
    //! STEP 5: Execute painting pattern
    long currentX = startX_steps;
    long currentY = startY_steps;
    long sweepYDistance = (long)(paintingSettings.getSide4SweepY() * STEPS_PER_INCH_XYZ);
    long shiftXDistance = (long)(paintingSettings.getSide4ShiftX() * STEPS_PER_INCH_XYZ);
    long paint_x_speed = paintingSettings.getSide4PaintingXSpeed();
    long paint_y_speed = paintingSettings.getSide4PaintingYSpeed();

    Serial.println("Side 4 Pattern: Executing specific Y-sweeps and X-shift.");

    // Move to the calculated starting point for this specific pattern sequence
    // Original logic: currentX = startX + (3 * shiftXDistance); currentY = startY + sweepYDistance;
    // This positions for a pattern that effectively starts around P7 and does P7->P8, P8->P9_equiv, P9_equiv->P10_equiv
    currentX = startX_steps + (3 * shiftXDistance); // Use Side 4's shiftX. If negative, it moves left.
    currentY = startY_steps + sweepYDistance;     // Use Side 4's sweepY.
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);
    Serial.println("Side 4 Pattern: Moved to calculated start for its specific sequence.");

    // First active sweep: Y- direction
    Serial.println("Side 4 Pattern: First active sweep Y-");
    paintGun_ON();
    currentY -= sweepYDistance;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting ABORTED due to home command");
        return;
    }

    // Interconnecting shift: X direction
    Serial.println("Side 4 Pattern: Interconnecting shift X");
    currentX += shiftXDistance; // Uses Side 4's shiftX (can be positive or negative)
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);

    // Second active sweep: Y+ direction
    Serial.println("Side 4 Pattern: Second active sweep Y+");
    paintGun_ON();
    currentY += sweepYDistance;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting ABORTED due to home command");
        return;
    }
    // -- END OF SIDE 4 PATTERN --

    //! STEP 8: Raise to safe Z height
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);

    Serial.println("Side 4 Pattern Painting Completed");
}