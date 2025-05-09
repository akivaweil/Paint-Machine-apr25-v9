#include <Arduino.h>
#include "../../include/motors/XYZ_Movements.h"
#include "../../include/utils/settings.h"
#include "../../include/motors/Rotation_Motor.h"
#include "../../include/hardware/paintGun_Functions.h"
#include "../../include/hardware/pressurePot_Functions.h"
#include "../../include/settings/painting.h"         // For painting-specific constants (SIDE4_Z_HEIGHT etc.)
#include "../../include/persistence/PaintingSettings.h" // Include for accessing saved settings
#include <FastAccelStepper.h>
// #include "../../include/persistence/PaintingSettings.h" // REMOVED - Redundant and commented out
// #include "../../include/persistence/persistence.h"     // REMOVED - No longer needed as servo angle comes from PaintingSettings object
#include "../../include/motors/ServoMotor.h"         // For ServoMotor class
#include "../../include/web/Web_Dashboard_Commands.h" // For checkForHomeCommand

// External references to stepper motors
extern FastAccelStepper *stepperX;
extern FastAccelStepper *stepperY_Left;
extern FastAccelStepper *stepperZ;
extern ServoMotor myServo;
extern PaintingSettings paintingSettings; // Make sure global instance is accessible

//* ************************************************************************
//* ************************** SIDE 4 PAINTING ***************************
//* ************************************************************************

void paintSide4Pattern() {
    Serial.println("Starting Side 4 Pattern Painting");

    //! Load Servo Angle
    int servoAngle = paintingSettings.getServoAngleSide4();

    //! Set Servo Angle FIRST
    myServo.setAngle(servoAngle);
    Serial.println("Servo set to: " + String(servoAngle) + " degrees for Side 4 side");

    //! STEP 0: Turn on pressure pot
    PressurePot_ON();

    //! STEP 1: Move to side 4 painting Z height (NOW USING SIDE 2 PARAMS)
    long zPos = (long)(paintingSettings.getSide2ZHeight() * STEPS_PER_INCH_XYZ); // Was getSide4ZHeight
    long sideZPos = (long)(paintingSettings.getSide2SideZHeight() * STEPS_PER_INCH_XYZ); // Was getSide4SideZHeight

    moveToXYZ(stepperX->getCurrentPosition(), DEFAULT_X_SPEED,
              stepperY_Left->getCurrentPosition(), DEFAULT_Y_SPEED,
              sideZPos, DEFAULT_Z_SPEED);

    //! STEP 2: Rotate to the (former) side 2 position
    rotateToAngle(SIDE2_ROTATION_ANGLE); // Was SIDE4_ROTATION_ANGLE
    Serial.println("Rotated to (former) side 2 position [now Side 4's target]");

    //! STEP 3: Move to start position (P1 of former Side 2)
    long startX = (long)(paintingSettings.getSide2StartX() * STEPS_PER_INCH_XYZ); // Was getSide4StartX
    long startY = (long)(paintingSettings.getSide2StartY() * STEPS_PER_INCH_XYZ); // Was getSide4StartY
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    Serial.println("Moved to (former) side 2 pattern start position (P1) [now Side 4's start]");

    //! STEP 4: Lower to painting Z height
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);

    // -- START OF FORMER SIDE 2 PATTERN (ADAPTED FOR SIDE 4 SETTINGS) --
    //! STEP 5: Execute painting pattern (formerly Side 2's TEST MODE logic)
    long currentX = startX; // Will be updated by calculation
    long currentY = startY; // Will be updated by calculation
    long sweepYDistance = (long)(paintingSettings.getSide4SweepY() * STEPS_PER_INCH_XYZ); // USE SIDE 4 SETTINGS
    long shiftXDistance = (long)(paintingSettings.getSide4ShiftX() * STEPS_PER_INCH_XYZ); // USE SIDE 4 SETTINGS

    //! (Formerly Side 2's) TEST MODIFICATION: Calculate starting point for its last two sweeps (P6 equivalent for original Side 2)
    Serial.println("Side 4 Pattern (formerly Side 2 Logic): TEST MODE - Calculating start for its specific sequence (P6 Side 2 equiv.)");
    // Original Side 2 logic: currentX = startX - (2 * shiftXDistance); currentY = startY - sweepYDistance;
    // This implies Side 2's P6 is to the left and down from its P1.
    // We apply this arithmetic using Side 4's startX, startY, shiftXDistance, and sweepYDistance.
    currentX = startX - (2 * shiftXDistance); // Uses Side 4's shiftXDistance
    currentY = startY - sweepYDistance;      // Uses Side 4's sweepYDistance
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);
    Serial.println("Side 4 Pattern (formerly Side 2 Logic): TEST MODE - Moved to its specific sequence start point.");

    /* // Original Side 2's commented out first three sweeps and two shifts are NOT brought over explicitly,
       // as the request is to swap the *active* painting pattern code. The active Side 2 code starts after this comment.
    */

    // (Formerly Side 2's) Third shift: X- direction (P6 to P7 for original Side 2)
    Serial.println("Side 4 Pattern (formerly Side 2 Logic): First active shift X-");
    currentX -= shiftXDistance; // Uses Side 4's shiftXDistance
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // USE SIDE 4 SPEEDS

    // (Formerly Side 2's) Fourth sweep: Y+ direction (P7 to P8 for original Side 2)
    Serial.println("Side 4 Pattern (formerly Side 2 Logic): First active sweep Y+");
    paintGun_ON();
    currentY += sweepYDistance; // Uses Side 4's sweepYDistance
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // USE SIDE 4 SPEEDS
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting (formerly Side 2 Logic) ABORTED due to home command");
        return;
    }

    // (Formerly Side 2's) Fourth shift: X- direction (P8 to P9 for original Side 2)
    Serial.println("Side 4 Pattern (formerly Side 2 Logic): Second active shift X-");
    currentX -= shiftXDistance; // Uses Side 4's shiftXDistance
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // USE SIDE 4 SPEEDS

    // (Formerly Side 2's) Fifth sweep: Y- direction (P9 to P10 for original Side 2)
    Serial.println("Side 4 Pattern (formerly Side 2 Logic): Second active sweep Y-");
    paintGun_ON();
    currentY -= sweepYDistance; // Uses Side 4's sweepYDistance
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED); // USE SIDE 4 SPEEDS
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting (formerly Side 2 Logic) ABORTED due to home command");
        return;
    }
    // -- END OF FORMER SIDE 2 PATTERN --

    //! STEP 8: Raise to safe Z height
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);

    Serial.println("Side 4 Pattern Painting Completed");
}
