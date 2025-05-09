#include <Arduino.h>
#include "../../include/motors/XYZ_Movements.h"
#include "../../include/utils/settings.h"
#include "../../include/motors/Rotation_Motor.h"
#include "../../include/hardware/paintGun_Functions.h"
#include "../../include/hardware/pressurePot_Functions.h"
#include <FastAccelStepper.h>
#include "../../include/settings/painting.h"
// #include "../../include/persistence/persistence.h" // Removed as unused and causing linter error
#include "../../include/motors/ServoMotor.h"
#include "../../include/persistence/PaintingSettings.h"
#include "../../include/web/Web_Dashboard_Commands.h"

// External references to stepper motors
extern FastAccelStepper *stepperX;
extern FastAccelStepper *stepperY_Left;
extern FastAccelStepper *stepperZ;
extern ServoMotor myServo;
extern PaintingSettings paintingSettings; // Make sure global instance is accessible

//* ************************************************************************
//* ************************** SIDE 2 PAINTING **************************
//* ************************************************************************
//* SIDE 2 SIDE PAINTING PATTERN
//*
//*    P9                      P7                      P5                      P3                      P1 (SIDE2_START_X,Y)
//*          ↓                     ↑                       ↓                       ↑                       ↓
//*          |                     |                       |                       |                       |
//*          |                     |                       |                       |                       |
//*          |                     |                       |                       |                       |
//*          |                     |                       |                       |                       |
//*          |                     |                       |                       |                       |
//*          |                     |                       |                       |                       |
//*          |                     |                       |                       |                       |
//*          |                     |                       |                       |                       |
//*          |                     |                       |                       |                       |
//*    P10 (END)               P8                      P6                      P4                      P2
//*          ←--------------------- ←--------------------- ←--------------------- ←--------------------- ←
//*
//* Sequence: 1→2→3→4→5→6→7→8→9→10 (Paint ON during vertical movements)
//*

// Function to paint the side 2 pattern
void paintSide2Pattern() {
    Serial.println("Starting Side 2 Pattern Painting");

    //! Load Servo Angle
    // persistence.begin(); // REMOVED
    int servoAngle = paintingSettings.getServoAngleSide2(); // NEW WAY: Use getter
    // float startX = persistence.loadFloat(KEY_START_X SIDE_2, 0.0);
    // float startY = persistence.loadFloat(KEY_START_Y SIDE_2, 0.0);
    // ... load other needed settings ...
    // persistence.end(); // REMOVED

    //! Set Servo Angle FIRST
    myServo.setAngle(servoAngle);
    Serial.println("Servo set to: " + String(servoAngle) + " degrees for Side 2 side");

    //! STEP 0: Turn on pressure pot
    PressurePot_ON();

    //! STEP 1: Move to side 2 painting Z height (NOW USING SIDE 4 PARAMS)
    long zPos = (long)(paintingSettings.getSide4ZHeight() * STEPS_PER_INCH_XYZ); // Was getSide2ZHeight
    long sideZPos = (long)(paintingSettings.getSide4SideZHeight() * STEPS_PER_INCH_XYZ); // Was getSide2SideZHeight

    moveToXYZ(stepperX->getCurrentPosition(), DEFAULT_X_SPEED,
              stepperY_Left->getCurrentPosition(), DEFAULT_Y_SPEED,
              sideZPos, DEFAULT_Z_SPEED);

    //! STEP 2: Rotate to the (former) side 4 position
    rotateToAngle(SIDE4_ROTATION_ANGLE); // Was SIDE2_ROTATION_ANGLE
    Serial.println("Rotated to (former) side 4 position [now Side 2's target]");

    //! STEP 3: Move to start position (P1 of former Side 4)
    long startX = (long)(paintingSettings.getSide4StartX() * STEPS_PER_INCH_XYZ); // Was getSide2StartX
    long startY = (long)(paintingSettings.getSide4StartY() * STEPS_PER_INCH_XYZ); // Was getSide2StartY
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    Serial.println("Moved to (former) side 4 pattern start position (P1) [now Side 2's start]");

    //! STEP 4: Lower to painting Z height
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);

    // -- START OF FORMER SIDE 4 PATTERN (ADAPTED FOR SIDE 2 SETTINGS) --
    //! STEP 5: Execute painting pattern (formerly Side 4's TEST MODE logic)
    long currentX = startX; // Will be updated by calculation
    long currentY = startY; // Will be updated by calculation
    long sweepYDistance = (long)(paintingSettings.getSide2SweepY() * STEPS_PER_INCH_XYZ); // USE SIDE 2 SETTINGS
    long shiftXDistance = (long)(paintingSettings.getSide2ShiftX() * STEPS_PER_INCH_XYZ); // USE SIDE 2 SETTINGS

    Serial.println("Side 2 Pattern (formerly Side 4 Logic): TEST MODE - Executing specific Y-sweeps and X-shift.");

    //! Calculate starting point for the (formerly Side 4's) 4th sweep equivalent
    // Original Side 4 P-notation:
    // P1: (startX, startY)
    // P2: (startX, startY + sweepY)
    // P3: (startX + shiftX, startY + sweepY)
    // P4: (startX + shiftX, startY)
    // P5: (startX + 2*shiftX, startY)
    // P6: (startX + 2*shiftX, startY + sweepY)
    // P7: (startX + 3*shiftX, startY + sweepY)
    // Note: For Side 2, X shifts are typically negative. Side 4's pattern used positive shifts.
    // We will retain the *structure* of Side 4's pattern but use Side 2's shift magnitudes/directions
    // and start points. The original Side 4 logic was: currentX = startX + (3 * shiftXDistance);
    // If Side 2's shiftXDistance is negative, this will correctly move left.
    // However, Side 4's pattern is inherently rightward-moving. If Side 2's area is to the left,
    // this might paint off the part or in an unexpected way.
    // For a direct logic swap, we keep the arithmetic operations from Side 4.
    currentX = startX + (3 * shiftXDistance); // Uses Side 2's shiftXDistance
    currentY = startY + sweepYDistance;     // Uses Side 2's sweepYDistance
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);
    Serial.println("Side 2 Pattern (formerly Side 4 Logic): TEST MODE - Moved to calculated start for its specific sequence.");

    // (Formerly Side 4's) "Fourth" sweep: Y- direction
    Serial.println("Side 2 Pattern (formerly Side 4 Logic): First active sweep Y-");
    paintGun_ON();
    currentY -= sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // USE SIDE 2 SPEEDS
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting (formerly Side 4 Logic) ABORTED due to home command");
        return;
    }

    // (Formerly Side 4's) "Fourth" shift: X+ direction
    Serial.println("Side 2 Pattern (formerly Side 4 Logic): Interconnecting shift X+");
    currentX += shiftXDistance; // This will use Side 2's shiftXDistance (potentially negative)
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // USE SIDE 2 SPEEDS

    // (Formerly Side 4's) "Fifth" sweep: Y+ direction
    Serial.println("Side 2 Pattern (formerly Side 4 Logic): Second active sweep Y+");
    paintGun_ON();
    currentY += sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // USE SIDE 2 SPEEDS
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting (formerly Side 4 Logic) ABORTED due to home command");
        return;
    }
    // -- END OF FORMER SIDE 4 PATTERN --

    //! STEP 8: Raise to safe Z height
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);

    Serial.println("Side 2 Pattern Painting Completed");
}
