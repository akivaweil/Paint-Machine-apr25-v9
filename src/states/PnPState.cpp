#include "states/PnPState.h"
#include <Arduino.h>
// #include "system/machine_state.h" // No longer needed
#include "utils/settings.h" 
#include "motors/XYZ_Movements.h"
#include "motors/stepper_globals.h" 
#include "hardware/cylinder_Functions.h"
#include "hardware/vacuum_Functions.h"
#include "motors/ServoMotor.h"
#include "system/StateMachine.h"
// #include "persistence/PnPData.h" // File does not exist
#include "states/IdleState.h" // Include IdleState for transition
#include "motors/Homing.h" // For homeAllAxes if needed, or a HomingState
#include "states/HomingState.h" // Include HomingState

// Reference to the global state machine instance (already declared as extern in PnPState.h)
// extern StateMachine* stateMachine; 

// Assume stepper motors are globally accessible as in the original file
extern FastAccelStepperEngine engine;
extern FastAccelStepper *stepperX;
extern FastAccelStepper *stepperY_Left;
extern FastAccelStepper *stepperY_Right;


//* ************************************************************************
//* ************************** PnP STATE **********************************
//* ************************************************************************

PnPState::PnPState() : 
    currentPnPGridPosition(0), 
    pnpCycleIsComplete(false), 
    pnp_step(0), 
    lastCycleTime(0),
    pickLocationX_steps(0), // Added for storing pick location
    pickLocationY_steps(0)  // Added for storing pick location
{
    // Constructor: Initialize members. Debouncer needs attach in enter().
    calculateGridPositions();
    // initializeHardware(); // Called in enter() instead

    // Calculate pick location in steps
    pickLocationX_steps = (long)(PICK_LOCATION_X * STEPS_PER_INCH_XYZ);
    pickLocationY_steps = (long)(PICK_LOCATION_Y * STEPS_PER_INCH_XYZ);
    Serial.printf("PnPState Constructed. Pick Location Steps: X=%ld, Y=%ld\n", pickLocationX_steps, pickLocationY_steps);

    // Don't reset state variables or move here, do it in enter()
}

void PnPState::enter() {
    Serial.println("Entering PnP State...");
    // setMachineState(MachineState::PNP); // REMOVED

    // Initialize PnP specific things
    calculateGridPositions();
    initializeHardware();

    // Calculate pick location in steps
    pickLocationX_steps = (long)(PICK_LOCATION_X * STEPS_PER_INCH_XYZ);
    pickLocationY_steps = (long)(PICK_LOCATION_Y * STEPS_PER_INCH_XYZ);
    Serial.printf("Pick Location Steps: X=%ld, Y=%ld\n", pickLocationX_steps, pickLocationY_steps);

    // Reset state variables
    // currentPnPGridPosition = 0; // Original starting position
    pnpCycleIsComplete = false;
    lastCycleTime = millis();  // Set initial cycle time
    pnp_step = 0; // Start with initial move to pick location

    // --- TEMPORARY MODIFICATION: Only process bottom two rows ---
    int startRow = GRID_ROWS - 2;
    if (startRow < 0) { // Ensure startRow is not negative if GRID_ROWS < 2
        startRow = 0;
    }
    currentPnPGridPosition = startRow * GRID_COLS;
    int totalOriginalPositions = GRID_ROWS * GRID_COLS;
    int endPositionForThisRun = totalOriginalPositions -1; 

    Serial.println("!!! TEMPORARY PnP MODE ACTIVE !!!");
    if (GRID_ROWS < 2 && GRID_ROWS > 0) {
        Serial.printf("NOTE: Processing only the single available row (Row 0), positions %d to %d.\n", currentPnPGridPosition, endPositionForThisRun);
    } else if (GRID_ROWS <= 0) {
        Serial.println("NOTE: No rows to process in PnP.");
         pnpCycleIsComplete = true; // No positions to process
    } else {
        Serial.printf("NOTE: Temporarily processing only bottom two rows (starting at Row %d, positions %d to %d of %d total original positions).\n", startRow, currentPnPGridPosition, endPositionForThisRun, totalOriginalPositions);
    }
    Serial.println("Original full grid processing is effectively commented out by starting later.");
    // --- END TEMPORARY MODIFICATION ---


    // --- Initiate FIRST Move to Pick Location (Non-Blocking) ---
    Serial.println("Initiating initial move to pick location...");
    moveToPickLocation(true); // Mark as initial move, starts non-blocking move

    Serial.println("PnP State Setup Complete. Initial move started.");
    // Original Note: Serial.println("NOTE: Starting at position 0 (top-right at grid origin) and working towards bottom-left (position 19).");
}

void PnPState::update() {
    // --- Check for Completion and Transition ---
    if (pnpCycleIsComplete && pnp_step == 4) {
        // PnP Cycle is fully finished, including the final move back to pick.
        Serial.println("PnP cycle fully complete!");

        // Stop any potential motor movement just in case before transitioning
        if (stepperX && stepperX->isRunning()) stepperX->stopMove();
        if (stepperY_Left && stepperY_Left->isRunning()) stepperY_Left->stopMove();
        if (stepperY_Right && stepperY_Right->isRunning()) stepperY_Right->stopMove();
        delay(50); // Small delay to ensure stops register

        // --- TRANSITION TO HOMING ---
        Serial.println("Transitioning to Homing State...");
        if (stateMachine) { // Check pointer
            stateMachine->changeState(stateMachine->getHomingState()); // Transition to Homing
        } else {
            Serial.println("ERROR: StateMachine pointer is null in PnPState! Cannot transition.");
        }
        return; // Exit update immediately after initiating transition
    }

    // Update the switch debouncer at the start of each update cycle
    pnpCycleSwitchDebouncer.update();
    bool switchIsActive = pnpCycleSwitchDebouncer.read() == HIGH;
    
    // --- PnP State Machine Logic ---
    switch (pnp_step) {
        case 0: // Moving to initial pick location
            if (stepperX && stepperY_Left && stepperY_Right && // Ensure steppers are valid
                !stepperX->isRunning() &&
                !stepperY_Left->isRunning() &&
                !stepperY_Right->isRunning())
            {
                Serial.println("Initial move complete. Reached pick location.");
                Serial.println("Now waiting for cycle switch press...");
                pnp_step = 1; // Transition to waiting state
            }
            // Otherwise, still moving, do nothing else this loop iteration
            break;

        case 1: // Waiting at Pick Location for Switch Press
            if (switchIsActive) {
                // Check if enough time has passed since the last cycle
                if ((millis() - lastCycleTime) >= 10) { // Use 10ms minimum delay for now
                    Serial.println("Cycle switch pressed at pick location. Timeout elapsed, proceeding...");

                    // Check if all positions were already completed
                    if (currentPnPGridPosition >= (GRID_ROWS * GRID_COLS)) {
                        Serial.println("All positions already completed. Setting complete flag.");
                        pnpCycleIsComplete = true;
                        pnp_step = 4; // Go to completion state
                    } else {
                        Serial.printf("Proceeding to process cycle for position %d.\n", currentPnPGridPosition);
                        pnp_step = 2; // Move to processing state
                    }
                } else {
                    // Serial.println("Switch active, but timeout not elapsed. Please wait."); // Optional: can be spammy
                    // Continue waiting until timeout
                }
            }
            // If switch not pressed, do nothing, stay in step 1.
            break;

        case 2: // Process Single PnP Cycle (Blocking Pick->Move->Place)
            // Check bounds just in case before processing
            if (currentPnPGridPosition < 0 || currentPnPGridPosition >= (GRID_ROWS * GRID_COLS)) {
                Serial.printf("ERROR: Invalid currentPnPGridPosition before processing cycle: %d\n", currentPnPGridPosition);
                pnpCycleIsComplete = true; // Force exit
                pnp_step = 4;
                break; // Exit switch statement
            }

            // --- Start the Blocking Cycle for the current position ---
            Serial.printf("Starting PnP cycle for position %d...\n", currentPnPGridPosition);
            process_single_pnp_cycle(); // This function blocks until done

            // --- Cycle Finished for this position ---
            currentPnPGridPosition++; // Increment position *after* processing
            Serial.printf("Cycle actions complete. Next logical position is %d.\n", currentPnPGridPosition);
            
            // Update the last cycle timestamp
            lastCycleTime = millis();

            // --- Decision Point: Check if all positions are now completed ---
            if (currentPnPGridPosition >= (GRID_ROWS * GRID_COLS)) {
                Serial.println("All PnP positions are now completed.");
                pnpCycleIsComplete = true;
                // Move back to pick location one last time (non-blocking) before completing
                Serial.println("Moving back to pick location before exiting...");
                moveToPickLocation(false); // Initiate non-blocking move back to pick
                pnp_step = 3; // Go to 'returning to pick' state before the final completion state
            } else {
                // Move back to pick location to wait for next cycle switch press
                Serial.println("Moving back to pick location...");
                moveToPickLocation(false); // Initiate non-blocking move back to pick
                pnp_step = 3; // Set state to 'moving back to pick location'
            }
            break; // End case 2

        case 3: // Moving back to pick location (after cycle completion or before final exit)
             if (stepperX && stepperY_Left && stepperY_Right && // Ensure steppers are valid
                !stepperX->isRunning() &&
                !stepperY_Left->isRunning() &&
                !stepperY_Right->isRunning())
            {
                if (pnpCycleIsComplete) {
                     Serial.println("Returned to pick location. PnP process complete.");
                     pnp_step = 4; // Transition to completion state
                } else {
                     Serial.println("Returned to pick location.");
                     Serial.println("Now waiting for cycle switch press for next position...");
                     pnp_step = 1; // Transition back to waiting state
                }
            }
            // Otherwise, still moving, do nothing else this loop iteration
            break;

        case 4: // Completion state
            // Logic to transition out is handled at the very beginning of update()
            // This state now mainly exists to ensure the final move back completes
            // before the transition out check happens.
            break;

        default:
            Serial.printf("ERROR: Invalid pnp_step: %d\n", pnp_step);
            // Attempt recovery? Maybe force to pick location or complete?
            pnpCycleIsComplete = true;
            moveToPickLocation(false); // Try to return to pick
            pnp_step = 3; // Go to return state as a safe default before completing
            break;
    }
}


void PnPState::exit() {
    // setMachineState(MachineState::UNKNOWN); // REMOVED
    Serial.println("Exiting PnP State");
    // Restore speeds if they were changed
    // Turn off any PnP specific indicators
}

const char* PnPState::getName() const {
    return "PNP";
}

// --- Private Helper Methods ---

void PnPState::calculateGridPositions() {
    Serial.println("Calculating grid positions...");
    const float X_SHIFT = 4.7f; // Inches between columns
    const float Y_SHIFT = 5.0f;  // Inches between rows
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            int index = row * GRID_COLS + col;
            gridPositionsX[index] = GRID_ORIGIN_X - (col * X_SHIFT);
            gridPositionsY[index] = GRID_ORIGIN_Y - (row * Y_SHIFT);
        }
    }
    Serial.println("Grid positions calculated.");
    // Optional: Print grid for verification
    // for(int i=0; i<GRID_ROWS*GRID_COLS; ++i) {
    //     Serial.printf("Pos %d: (%.2f, %.2f)\n", i, gridPositionsX[i], gridPositionsY[i]); // Fixed printf
    // }
}

void PnPState::initializeHardware() {
    Serial.println("Initializing PnP Hardware...");
    // Initialize cycle switch
    pnpCycleSwitchDebouncer.attach(PNP_CYCLE_SWITCH_PIN, INPUT_PULLDOWN);
    pnpCycleSwitchDebouncer.interval(20); // 20ms debounce
    Serial.println("Cycle switch initialized.");

    // Initialize hardware to safe state
    vacuumOff();
    cylinderUp();
    Serial.println("Vacuum off, Cylinder up.");
}

// Initiates a non-blocking move to the pre-calculated pick location
void PnPState::moveToPickLocation(bool initialMove) {
    Serial.print(initialMove ? "Initiating initial move" : "Moving back");
    Serial.println(" to pick location...");
    
    if (stepperX && stepperY_Left && stepperY_Right) {
        stepperX->setAcceleration(DEFAULT_X_ACCEL);
        stepperY_Left->setAcceleration(DEFAULT_Y_ACCEL);
        stepperY_Right->setAcceleration(DEFAULT_Y_ACCEL);
        stepperX->setSpeedInHz(DEFAULT_X_SPEED);
        stepperY_Left->setSpeedInHz(DEFAULT_Y_SPEED);
        stepperY_Right->setSpeedInHz(DEFAULT_Y_SPEED);

        stepperX->moveTo(pickLocationX_steps);
        stepperY_Left->moveTo(pickLocationY_steps); 
        stepperY_Right->moveTo(pickLocationY_steps); 
        
        pnp_step = initialMove ? 0 : 3; // Set state to 'moving to pick'
    } else {
        Serial.println("ERROR: Steppers not initialized! Cannot move to pick location.");
        pnpCycleIsComplete = true; // Prevent operation
        pnp_step = 4; // Go directly to 'complete' state to signal error/need for reset
    }
}

// Performs actions for the current 'currentPnPGridPosition' (BLOCKING)
void PnPState::process_single_pnp_cycle() {
    // Check bounds just in case
    if (currentPnPGridPosition < 0 || currentPnPGridPosition >= (GRID_ROWS * GRID_COLS)) {
         Serial.printf("ERROR: Invalid currentPnPGridPosition in process_single_pnp_cycle: %d\n", currentPnPGridPosition);
         pnpCycleIsComplete = true; // Force exit in update loop
         // Don't set pnp_step here, let the update loop handle error transition
         return; // Exit this function early
    }

    // Ensure motors are stopped before starting blocking moves (should be already stopped if coming from wait state)
    if (stepperX && stepperX->isRunning()) stepperX->stopMove();
    if (stepperY_Left && stepperY_Left->isRunning()) stepperY_Left->stopMove();
    if (stepperY_Right && stepperY_Right->isRunning()) stepperY_Right->stopMove();
    delay(50); // Small delay to ensure motors fully stopped

    Serial.printf("Processing PnP position %d (%d of %d)\n",
                  currentPnPGridPosition, currentPnPGridPosition + 1, GRID_ROWS * GRID_COLS);

    //! STEP 1: Confirm already at pick location (or move if somehow drifted - should not happen in normal flow)
    // Since we always return to pick location now, this move *should* be instantaneous or very small
    // Can be removed if confident state machine always ensures starting at pick location
    Serial.println("Verifying at pick location...");
     moveToXYZ(
         pickLocationX_steps, DEFAULT_X_SPEED,
         pickLocationY_steps, DEFAULT_Y_SPEED,
         0, DEFAULT_Z_SPEED // Z assumed at home/0
     );
    Serial.println("Confirmed at pick location.");


    //! STEP 2: Extend cylinder, activate vacuum, retract cylinder
    Serial.println("Extending cylinder...");
    cylinderDown();
    delay(PNP_CYLINDER_EXTEND_TIME);
    Serial.println("Activating vacuum...");
    vacuumOn();
    delay(PNP_VACUUM_ENGAGE_DELAY);
    Serial.println("Retracting cylinder...");
    cylinderUp();
    delay(PNP_CYLINDER_RETRACT_TIME);
    Serial.println("Pick sequence complete.");

    //! STEP 3: Get target grid position coordinates
    float targetX_inch = gridPositionsX[currentPnPGridPosition];
    float targetY_inch = gridPositionsY[currentPnPGridPosition];
    long targetX_steps = (long)(targetX_inch * STEPS_PER_INCH_XYZ);
    long targetY_steps = (long)(targetY_inch * STEPS_PER_INCH_XYZ);

    //! STEP 4: Move to place position (Blocking)
    int row = currentPnPGridPosition / GRID_COLS;
    int col = currentPnPGridPosition % GRID_COLS;
    Serial.printf("Moving to grid position [%d][%d] (%d): X=%.2f (%ld steps), Y=%.2f (%ld steps)\n",
                  row, col, currentPnPGridPosition, targetX_inch, targetX_steps, targetY_inch, targetY_steps);

    moveToXYZ(
        targetX_steps, DEFAULT_X_SPEED,
        targetY_steps, DEFAULT_Y_SPEED,
        0, DEFAULT_Z_SPEED // Z assumed at home/0
    );
     Serial.println("Arrived at place location.");

    //! STEP 5: Extend cylinder, deactivate vacuum, retract cylinder
    Serial.println("Extending cylinder to place component...");
    cylinderDown();
    delay(PNP_CYLINDER_EXTEND_TIME);
    Serial.println("Deactivating vacuum...");
    vacuumOff();
    delay(PNP_VACUUM_ENGAGE_DELAY); // Using same delay for release
    Serial.println("Retracting cylinder...");
    cylinderUp();
    delay(PNP_CYLINDER_RETRACT_TIME);

    Serial.println("Place sequence complete. Cycle finished, machine is at Place Location.");
    Serial.println("------------------------------------");
    // NOTE: Function ends here. The machine is currently at the place location.
    // The state machine (update loop) will now initiate the move back to the pick location.
} 