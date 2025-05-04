#ifndef SETTINGS_PINS_H
#define SETTINGS_PINS_H

// ==========================================================================
//                            PIN DEFINITIONS
// ==========================================================================

// --- Stepper Motor Pins ---
// X Axis
#define X_STEP_PIN 36
#define X_DIR_PIN 35
#define X_HOME_SWITCH 5

// Y Axis Left
#define Y_LEFT_STEP_PIN 42
#define Y_LEFT_DIR_PIN 41
#define Y_LEFT_HOME_SWITCH 16

// Y Axis Right (Note: Y-Right Homing Switch only, shares Y-Left Motor pins)
#define Y_RIGHT_HOME_SWITCH 18

// Z Axis
#define Z_STEP_PIN 38
#define Z_DIR_PIN 37
#define Z_HOME_SWITCH 4

// Rotation Axis (Optional Homing)
#define ROTATION_STEP_PIN 40
#define ROTATION_DIR_PIN 39

// --- Servo & Actuator Pins ---
// Pitch Servo Removed
#define PICK_CYLINDER_PIN 10
#define SUCTION_PIN 11
#define PAINT_GUN_PIN 12
#define PRESSURE_POT_PIN 13

// --- Control Input Pins ---
#define PNP_CYCLE_SWITCH_PIN 17

#endif // SETTINGS_PINS_H 