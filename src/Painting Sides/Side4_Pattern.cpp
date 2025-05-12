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
//* SIDE 4 PAINTING PATTERN (Vertical Sweeps) - NEW LOGIC
//* Based on original Side 2 pattern (5-sweep, starts +Y, shifts +X)
//*
//*    P1 (SIDE4_START_X,Y)         P3                      P5                      P7                      P9
//*          ↓ (+Y Start Sweep)      ↑ (-Y)                  ↓ (+Y)                  ↑ (-Y)                  ↓ (+Y)
//*    P2                      P4                      P6                      P8                      P10 (END)
//*          → (+X shift)          → (+X shift)          → (+X shift)          → (+X shift)
//*
//* Pattern: Start at (Side4_StartX, Side4_StartY). Sweep +Y. Shift +X. Sweep -Y. Shift +X. Sweep +Y ... for 5 Y sweeps.    

void paintSide4Pattern() {
    Serial.println("Starting Side 4 Pattern Painting (New Logic - Swapped from Side 2)");

    int servoAngle = paintingSettings.getServoAngleSide4(); // Use Side 4 settings
    long zPos = (long)(paintingSettings.getSide4ZHeight() * STEPS_PER_INCH_XYZ); // Use Side 4 settings
    long sideZPos = (long)(paintingSettings.getSide4SideZHeight() * STEPS_PER_INCH_XYZ); // Use Side 4 settings
    long startX_steps = (long)(paintingSettings.getSide4StartX() * STEPS_PER_INCH_XYZ); // Use Side 4 settings
    long startY_steps = (long)(paintingSettings.getSide4StartY() * STEPS_PER_INCH_XYZ); // Use Side 4 settings
    long sweepYDistance = (long)(paintingSettings.getSide4SweepY() * STEPS_PER_INCH_XYZ); // Use Side 4 settings
    long shiftXDistance = (long)(paintingSettings.getSide4ShiftX() * STEPS_PER_INCH_XYZ); // Use Side 4 settings - ensure this is positive for +X shift
    long paint_x_speed = paintingSettings.getSide4PaintingXSpeed(); // Use Side 4 settings
    long paint_y_speed = paintingSettings.getSide4PaintingYSpeed(); // Use Side 4 settings
    long initial_sweep_paint_y_speed_side4 = (long)(paint_y_speed * 0.75f); // Renamed from final_sweep_paint_y_speed_side4

    //! Set Servo Angle FIRST
    myServo.setAngle(servoAngle);
    Serial.println("Servo set to: " + String(servoAngle) + " degrees for Side 4");

    //! STEP 0: Turn on pressure pot
    PressurePot_ON();

    //! STEP 1: Move to Side 4 safe Z height at current X,Y
    moveToXYZ(stepperX->getCurrentPosition(), DEFAULT_X_SPEED,
              stepperY_Left->getCurrentPosition(), DEFAULT_Y_SPEED,
              sideZPos, DEFAULT_Z_SPEED);

    //! STEP 2: Rotate to the Side 4 position
    rotateToAngle(SIDE4_ROTATION_ANGLE); // Use Side 4 angle
    Serial.println("Rotated to Side 4 position");

    //! STEP 3: Move to user-defined start X, Y for Side 4 at safe Z height
    moveToXYZ(startX_steps, DEFAULT_X_SPEED, startY_steps, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
    Serial.println("Moved to Side 4 Start X, Y at safe Z.");

    //! STEP 4: Lower to painting Z height
    moveToXYZ(startX_steps, DEFAULT_X_SPEED, startY_steps, DEFAULT_Y_SPEED, zPos, DEFAULT_Z_SPEED);
    Serial.println("Lowered to painting Z for Side 4.");

    long currentX = startX_steps;
    long currentY = startY_steps;
    const int num_y_sweeps = 5; // Corresponds to P1-P10 in the diagram if using 4 shifts

    for (int i = 0; i < num_y_sweeps; ++i) {
        bool isPositiveYSweep = (i % 2 == 0); // 0th, 2nd, 4th sweeps are +Y
        long current_paint_y_speed = paint_y_speed;

        if (i == 0) { // First sweep (changed from i == num_y_sweeps - 1)
            current_paint_y_speed = initial_sweep_paint_y_speed_side4;
            Serial.printf("Side 4 Pattern: Applying 75%% speed for first sweep: %ld\\n", current_paint_y_speed);
        }

        if (isPositiveYSweep) {
            Serial.printf("Side 4 Pattern: Sweep %d (+Y)\\\n", i + 1);
            paintGun_ON();
            currentY += sweepYDistance;
            moveToXYZ(currentX, paint_x_speed, currentY, current_paint_y_speed, zPos, DEFAULT_Z_SPEED);
            paintGun_OFF();
        } else {
            Serial.printf("Side 4 Pattern: Sweep %d (-Y)\\\n", i + 1);
            paintGun_ON();
            currentY -= sweepYDistance;
            moveToXYZ(currentX, paint_x_speed, currentY, current_paint_y_speed, zPos, DEFAULT_Z_SPEED);
            paintGun_OFF();
        }

        if (checkForHomeCommand()) {
            moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
            Serial.println("Side 4 Pattern Painting ABORTED due to home command");
            return;
        }

        // Perform X shift if it's not the last Y sweep
        if (i < num_y_sweeps - 1) {
            Serial.printf("Side 4 Pattern: Shift +X after sweep %d\\\\n", i + 1);
            currentX += shiftXDistance; // Shift in +X direction (ensure shiftXDistance is positive in settings for +X)
            moveToXYZ(currentX, paint_x_speed, currentY, paint_y_speed, zPos, DEFAULT_Z_SPEED); // Use original paint_y_speed for X shift
            
            if (checkForHomeCommand()) {
                 moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);
                 Serial.println("Side 4 Pattern Painting ABORTED due to home command during X shift");
                 return;
            }
        }
    }

    //! Raise to safe Z height after completing the pattern
    moveToXYZ(currentX, DEFAULT_X_SPEED, currentY, DEFAULT_Y_SPEED, sideZPos, DEFAULT_Z_SPEED);

    Serial.println("Side 4 Pattern Painting Completed (New Logic - Swapped from Side 2)");
}