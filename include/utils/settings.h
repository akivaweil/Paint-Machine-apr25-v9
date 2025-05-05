#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h> // Include for uint32_t type

// Include all settings modules
#include "settings/pins.h"
#include "settings/motion.h"
#include "settings/homing.h"
#include "settings/timing.h"
#include "settings/painting.h"
#include "settings/pnp.h"

// Y Axis Left
#define Y_LEFT_DIR_PIN 41
#define Y_LEFT_HOME_SWITCH 16

// Y Axis Right
#define Y_RIGHT_STEP_PIN 1
#define Y_RIGHT_DIR_PIN 2
#define Y_RIGHT_HOME_SWITCH 18

// Z Axis

#endif // SETTINGS_H 