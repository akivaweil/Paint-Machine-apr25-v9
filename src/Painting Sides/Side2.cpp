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

    //! STEP 1: Move to side 2 painting Z height
    long zPos = (long)(paintingSettings.getSide2ZHeight() * STEPS_PER_INCH_XYZ);
    long sideZPos = (long)(paintingSettings.getSide2SideZHeight() * STEPS_PER_INCH_XYZ);

    moveToXYZ(stepperX->getCurrentPosition(), DEFAULT_X_SPEED,
              stepperY_Left->getCurrentPosition(), DEFAULT_Y_SPEED,
              sideZPos, DEFAULT_Z_SPEED);

    //! STEP 2: Rotate to the side 2 position
    rotateToAngle(SIDE2_ROTATION_ANGLE);
    Serial.println("Rotated to side 2 position");

    //! STEP 3: Move to start position (P1)
    long startX = (long)(paintingSettings.getSide2StartX() * STEPS_PER_INCH_XYZ);
    long startY = (long)(paintingSettings.getSide2StartY() * STEPS_PER_INCH_XYZ);
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    Serial.println("Moved to side 2 pattern start position (P1)");

    //! STEP 4: Lower to painting Z height
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);

    //! STEP 5: Execute side 2 painting pattern
    long currentX = startX;
    long currentY = startY;
    long sweepYDistance = (long)(paintingSettings.getSide2SweepY() * STEPS_PER_INCH_XYZ);
    long shiftXDistance = (long)(paintingSettings.getSide2ShiftX() * STEPS_PER_INCH_XYZ);

    //! TEST MODIFICATION: Calculate starting point for the last two sweeps (at P6)
    //! and move there. This skips the first 3 sweeps and 2 shifts.
    Serial.println("Side 2 Pattern: TEST MODE - Calculating start for last two sweeps (P6 equivalent)");
    currentX = startX - (2 * shiftXDistance); // Calculated X for P6
    currentY = startY - sweepYDistance;      // Calculated Y for P6
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);
    Serial.println("Side 2 Pattern: TEST MODE - Moved to P6 equivalent position.");

    /* // Original First three sweeps and two shifts - COMMENTED OUT FOR TEST
    // First sweep: Y- direction (P1 to P2)
    Serial.println("Side 2 Pattern: First sweep Y-");
    paintGun_ON();
    currentY -= sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting ABORTED due to home command");
        return;
    }

    // First shift: X- direction (P2 to P3)
    Serial.println("Side 2 Pattern: Shift X-");
    currentX -= shiftXDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);

    // Second sweep: Y+ direction (P3 to P4)
    Serial.println("Side 2 Pattern: Second sweep Y+");
    paintGun_ON();
    currentY += sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting ABORTED due to home command");
        return;
    }

    // Second shift: X- direction (P4 to P5)
    Serial.println("Side 2 Pattern: Shift X-");
    currentX -= shiftXDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);

    // Third sweep: Y- direction (P5 to P6)
    Serial.println("Side 2 Pattern: Third sweep Y-");
    paintGun_ON();
    currentY -= sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting ABORTED due to home command");
        return;
    }
    */ // END OF COMMENTED OUT SECTION

    // Third shift: X- direction (P6 to P7)
    Serial.println("Side 2 Pattern: Shift X- (P6 to P7)"); // Clarified comment
    currentX -= shiftXDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);

    // Fourth sweep: Y+ direction (P7 to P8)
    Serial.println("Side 2 Pattern: Fourth sweep Y+ (P7 to P8)"); // Clarified comment
    paintGun_ON();
    currentY += sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting ABORTED due to home command");
        return;
    }

    // Fourth shift: X- direction (P8 to P9)
    Serial.println("Side 2 Pattern: Shift X- (P8 to P9)"); // Clarified comment
    currentX -= shiftXDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);

    // Fifth sweep: Y- direction (P9 to P10)
    Serial.println("Side 2 Pattern: Fifth sweep Y- (P9 to P10)"); // Clarified comment
    paintGun_ON();
    currentY -= sweepYDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 2 Pattern Painting ABORTED due to home command");
        return;
    }

    // Fifth shift: X- direction (after P10)
    Serial.println("Side 2 Pattern: Final shift X- (after P10)"); // Clarified comment
    currentX -= shiftXDistance;
    moveToXYZ(currentX, paintingSettings.getSide2PaintingXSpeed(), currentY, paintingSettings.getSide2PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);

    //! STEP 8: Raise to safe Z height
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);

    Serial.println("Side 2 Pattern Painting Completed");
}
