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

    //! STEP 1: Move to side 4 painting Z height
    long zPos = (long)(paintingSettings.getSide4ZHeight() * STEPS_PER_INCH_XYZ);
    long sideZPos = (long)(paintingSettings.getSide4SideZHeight() * STEPS_PER_INCH_XYZ);

    moveToXYZ(stepperX->getCurrentPosition(), DEFAULT_X_SPEED,
              stepperY_Left->getCurrentPosition(), DEFAULT_Y_SPEED,
              sideZPos, DEFAULT_Z_SPEED);

    //! STEP 2: Rotate to the side 4 position
    rotateToAngle(SIDE4_ROTATION_ANGLE);
    Serial.println("Rotated to side 4 position");

    //! STEP 3: Move to start position (P1)
    long startX = (long)(paintingSettings.getSide4StartX() * STEPS_PER_INCH_XYZ);
    long startY = (long)(paintingSettings.getSide4StartY() * STEPS_PER_INCH_XYZ);
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    Serial.println("Moved to side 4 pattern start position (P1)");

    //! STEP 4: Lower to painting Z height
    moveToXYZ(startX, DEFAULT_X_SPEED, startY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);

    //! STEP 5: Execute side 4 painting pattern (TEST MODE)
    long currentX = startX; // Will be updated by calculation
    long currentY = startY; // Will be updated by calculation
    long sweepYDistance = (long)(paintingSettings.getSide4SweepY() * STEPS_PER_INCH_XYZ);
    long shiftXDistance = (long)(paintingSettings.getSide4ShiftX() * STEPS_PER_INCH_XYZ);

    Serial.println("Side 4 Pattern: TEST MODE - Executing 4th and 5th Y-sweeps and interconnecting X-shift.");

    //! Calculate starting point for the 4th sweep (P7 equivalent)
    // P1: (startX, startY)
    // P2: (startX, startY + sweepY)
    // P3: (startX + shiftX, startY + sweepY)
    // P4: (startX + shiftX, startY)
    // P5: (startX + 2*shiftX, startY)
    // P6: (startX + 2*shiftX, startY + sweepY)
    // P7: (startX + 3*shiftX, startY + sweepY)
    currentX = startX + (3 * shiftXDistance);
    currentY = startY + sweepYDistance;
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED); // Move to calculated P7 at painting Z
    Serial.println("Side 4 Pattern: TEST MODE - Moved to calculated start position for 4th sweep (P7 equivalent).");

    // Fourth sweep: Y- direction (P7 to P8) - This is the 1st active sweep in test
    // currentX is startX + 3*shiftXDistance
    // currentY is startY + sweepYDistance
    Serial.println("Side 4 Pattern: Fourth sweep Y- (Active - 1st of 2 sweeps)");
    paintGun_ON();
    currentY -= sweepYDistance; // Target Y: startY
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting ABORTED due to home command (after 4th sweep)");
        return;
    }

    // Fourth shift: X+ direction (P8 to P9) - This is the 1st active shift in test
    // currentX is startX + 3*shiftXDistance
    // currentY is startY
    Serial.println("Side 4 Pattern: Shift X+ (Active - Interconnecting shift)");
    currentX += shiftXDistance; // Target X: startX + 4*shiftXDistance
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);

    // Fifth sweep: Y+ direction (P9 to P10) - This is the 2nd active sweep in test
    // currentX is startX + 4*shiftXDistance
    // currentY is startY
    Serial.println("Side 4 Pattern: Fifth sweep Y+ (Active - 2nd of 2 sweeps)");
    paintGun_ON();
    currentY += sweepYDistance; // Target Y: startY + sweepYDistance
    moveToXYZ(currentX, paintingSettings.getSide4PaintingXSpeed(), currentY, paintingSettings.getSide4PaintingYSpeed(), zPos, DEFAULT_Z_SPEED);
    paintGun_OFF();

    if (checkForHomeCommand()) {
        moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
        Serial.println("Side 4 Pattern Painting ABORTED due to home command (after 5th sweep)");
        return;
    }

    //! STEP 8: Raise to safe Z height
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);

    Serial.println("Side 4 Pattern Painting Completed");
}
