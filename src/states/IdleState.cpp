#include "states/IdleState.h"
#include <Arduino.h>
#include <Bounce2.h> // Include Bounce2 library
#include "system/machine_state.h"
#include "utils/settings.h" // Include for PNP_CYCLE_SWITCH_PIN
#include "system/StateMachine.h" // Include for state machine access
#include "states/PnPState.h" // Include the new PnPState

// Reference to the global state machine instance
extern StateMachine* stateMachine;

//* ************************************************************************
//* ***************************** IDLE STATE ******************************
//* ************************************************************************

Bounce pnpCycleSwitch; // Debouncer for the physical PnP cycle switch

IdleState::IdleState() {
    // Constructor implementation
}

void IdleState::enter() {
    Serial.println("Entering Idle State");
    // Set machine status or perform actions specific to entering idle
    setMachineState(MachineState::IDLE); // Update the global state enum

    // Stop motors if they were moving (safety measure)
    // stopAllMotors(); // Example function call

    // Ensure any indicators (like LEDs) show idle status
    // setIdleLED(true);

    // Initialize the PnP cycle switch debouncer
    pnpCycleSwitch.attach(PNP_CYCLE_SWITCH_PIN, INPUT_PULLDOWN);
    pnpCycleSwitch.interval(20); // 20ms debounce time

    Serial.println("Idle state active. Press PnP cycle switch to enter PnP mode."); 
}

void IdleState::update() {
    // Monitor for events that trigger state changes
    // e.g., check for button presses, web commands, sensor triggers

    // Example: Check for a start button press
    // if (digitalRead(START_BUTTON_PIN) == HIGH) {
    //     stateMachine->changeState(stateMachine->getHomingState()); 
    // }

    // Example: Check for incoming web command (handled elsewhere, but could be checked here)

    // Keep machine state updated if necessary (though likely done on entry)
    if (getMachineState() != MachineState::IDLE) {
         setMachineState(MachineState::IDLE); 
    }

    // No continuous actions typically occur in Idle

    // Update the debouncer
    pnpCycleSwitch.update();

    // Check if the PnP cycle switch was pressed (rising edge)
    if (pnpCycleSwitch.rose()) {
        Serial.println("PnP Cycle Switch pressed in Idle State. Transitioning to PnPState...");
        if (stateMachine) { // Check if stateMachine exists
            stateMachine->changeState(stateMachine->getPnpState()); // Use getter
        } else {
            Serial.println("ERROR: StateMachine pointer is null in IdleState!");
        }
        return; // Exit update early after transition
    }
}

void IdleState::exit() {
    Serial.println("Exiting Idle State");
    // Cleanup actions when leaving idle state
    // e.g., turn off idle indicators
    // setIdleLED(false);
}

const char* IdleState::getName() const {
    return "IDLE";
} 