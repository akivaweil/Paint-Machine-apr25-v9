#ifndef SETTINGS_PNP_H
#define SETTINGS_PNP_H

// ==========================================================================
//                     PICK AND PLACE / TRAY PARAMETERS
// ==========================================================================
// Defaults, can be changed via UI/saved settings

// --- Component/Tray Dimensions (inches) ---
#define SQUARE_WIDTH 3.0f                      // Square width
#define TRAY_BORDER_WIDTH 0.25f                // Tray border width
#define TRAY_WIDTH 18.0f                       // Tray width
#define TRAY_HEIGHT 24.0f                      // Tray height

// --- Grid Layout ---
#define GRID_COLS 4                            // Grid columns
#define GRID_ROWS 5                            // Grid rows
#define GRID_ORIGIN_X 18.5f                    // Grid origin X position (top-right corner)
#define GRID_ORIGIN_Y 32.25f                    // Grid origin Y position (top-right corner)

// --- Pick Location (inches) ---
#define PICK_LOCATION_X 12.5f                  // X coordinate for component pickup
#define PICK_LOCATION_Y 2.0f                   // Y coordinate for component pickup

// --- PNP Operation Delays (ms) ---
#define PNP_CYLINDER_EXTEND_TIME 300          // Delay after extending cylinder
#define PNP_VACUUM_ENGAGE_DELAY 300            // Delay after engaging vacuum
#define PNP_CYLINDER_RETRACT_TIME 250         // Delay after retracting cylinder before movement

// Timeout between allowed cycle switch presses
#define CYCLE_TIMEOUT 5 // milliseconds

#endif // SETTINGS_PNP_H 