#include <Arduino.h>
#include "../../include/motors/XYZ_Movements.h"       // For moveToXYZ
#include "../../include/utils/settings.h"         // For STEPS_PER_INCH, default speeds
#include "../../include/motors/Rotation_Motor.h"     // For rotateToAngle
#include "../../include/hardware/paintGun_Functions.h" // For paintGun_ON/OFF
#include "../../include/hardware/pressurePot_Functions.h" // For PressurePot_ON
#include "../../include/settings/painting.h"         // For painting-specific constants (SIDE3_Z_HEIGHT etc.)
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
extern PaintingSettings paintingSettings; // Make sure global instance is accessible

//* ************************************************************************
//* **************************** SIDE 3 PAINTING ****************************
//* ************************************************************************
//* SIDE 3 SIDE PAINTING PATTERN (Horizontal Sweeps)
//*
//* P1 (startX,startY)  ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ←  (startX-sweepX,startY)
//*       |                                                      |
//*       | Shift 1 (Y-)                                         |
//*       ↓                                                      |
//* (startX,startY-shiftY)  → → → → → → → → → → → → → → → →  (startX-sweepX,startY-shiftY)
//*       |                                                      |
//*       | Shift 2 (Y-)                                         |
//*       ↓                                                      |
//* (startX,startY-2*shiftY)← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ←  (startX-sweepX,startY-2*shiftY)
//*       |                                                      |
//*       | Shift 3 (Y-)                                         |
//*       ↓                                                      |
//* (startX,startY-3*shiftY)→ → → → → → → → → → → → → → → →  (startX-sweepX,startY-3*shiftY)
//*
//* Sequence: Start → Sweep X- → Shift Y- → Sweep X+ → Shift Y- → Sweep X- → Shift Y- → Sweep X+
//* Paint ON during horizontal (X) sweeps. Start position assumed to be top-right corner.
//*

// Function to paint the side 3 pattern
void paintSide3Pattern() {
    Serial.println("Starting Side 3 Pattern Painting (Horizontal Sweeps)");

    // Temporarily load settings required for this pattern
    // persistence.begin(); // REMOVED
    int servoAngle = paintingSettings.getServoAngleSide3(); // NEW WAY: Use getter
    // ... load other needed settings ...
    // persistence.end(); // REMOVED

    //! Set Servo Angle FIRST
    myServo.setAngle(servoAngle);
    Serial.println("Servo set to: " + String(servoAngle) + " degrees for Side 3 side");

    //! STEP 0: Turn on pressure pot
    PressurePot_ON();

    //! STEP 1: Move to side 3 painting Z height
    long zPos = (long)(paintingSettings.getSide3ZHeight() * STEPS_PER_INCH_XYZ); // Use getter
    long sideZPos = (long)(paintingSettings.getSide3SideZHeight() * STEPS_PER_INCH_XYZ); // Use getter

    moveToXYZ(stepperX->getCurrentPosition(), DEFAULT_X_SPEED,
              stepperY_Left->getCurrentPosition(), DEFAULT_Y_SPEED,
              sideZPos, DEFAULT_Z_SPEED);

    //! STEP 2: Rotate to the side 3 position
    rotateToAngle(SIDE3_ROTATION_ANGLE);
    Serial.println("Rotated to side 3 position");

    //! STEP 3: Move to start position (Top Right - P1 assumed)
    long startX_steps = (long)(paintingSettings.getSide3StartX() * STEPS_PER_INCH_XYZ); // Use getter
    long startY_steps = (long)(paintingSettings.getSide3StartY() * STEPS_PER_INCH_XYZ); // Use getter
    moveToXYZ(startX_steps, DEFAULT_X_SPEED, startY_steps, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    Serial.println("Moved to side 3 pattern start position (Top Right)");

    //! STEP 4: Lower to painting Z height
    moveToXYZ(startX_steps, DEFAULT_X_SPEED, startY_steps, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);

    //! STEP 5: Execute side 3 horizontal painting pattern
    long currentX = startX_steps;
    long currentY = startY_steps;
    // Calculate sweep/shift distances in steps based on settings
    // For horizontal pattern: Use Side3ShiftX for X-sweep length, Side3SweepY for Y-shift distance
    long sweepX_steps = (long)(paintingSettings.getSide3ShiftX() * STEPS_PER_INCH_XYZ); // Use getter for X sweep length
    long shiftY_steps = (long)(paintingSettings.getSide3SweepY() * STEPS_PER_INCH_XYZ); // Use getter for Y shift distance

    long paint_x_speed = paintingSettings.getSide3PaintingXSpeed(); // Use getters for speed
    long paint_y_speed = paintingSettings.getSide3PaintingYSpeed(); // Use getters for speed

    // First sweep: X- direction
    Serial.println("Side 3 Pattern: First sweep X-");
    paintGun_ON();
    currentX -= sweepX_steps;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 3 Pattern Painting ABORTED due to home command");
        return;
    }

    // First shift: Y- direction
    Serial.println("Side 3 Pattern: Shift Y-");
    currentY -= shiftY_steps;
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED); // Use default speeds for shifts

    // Second sweep: X+ direction
    Serial.println("Side 3 Pattern: Second sweep X+");
    paintGun_ON();
    currentX += sweepX_steps;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 3 Pattern Painting ABORTED due to home command");
        return;
    }

    // Second shift: Y- direction
    Serial.println("Side 3 Pattern: Shift Y-");
    currentY -= shiftY_steps;
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED); // Use default speeds for shifts

    // Third sweep: X- direction
    Serial.println("Side 3 Pattern: Third sweep X-");
    paintGun_ON();
    currentX -= sweepX_steps;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 3 Pattern Painting ABORTED due to home command");
        return;
    }

    // Third shift: Y- direction
    Serial.println("Side 3 Pattern: Shift Y-");
    currentY -= shiftY_steps;
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED); // Use default speeds for shifts

    // Fourth sweep: X+ direction
    Serial.println("Side 3 Pattern: Fourth sweep X+");
    paintGun_ON();
    currentX += sweepX_steps;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // Fourth shift: Y- direction
    Serial.println("Side 3 Pattern: Shift Y-");
    currentY -= shiftY_steps;
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED); // Use default speeds for shifts

    // Fifth sweep: X- direction
    Serial.println("Side 3 Pattern: Fifth sweep X-");
    paintGun_ON();
    currentX -= sweepX_steps;
    moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    // Check for home command after sweep
    if (checkForHomeCommand()) {
        // Raise to safe Z height before aborting
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 3 Pattern Painting ABORTED due to home command");
        return;
    }

    //! STEP 8: Raise to safe Z height
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);

    Serial.println("Side 3 Pattern Painting Completed");
}
