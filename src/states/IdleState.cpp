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
    // Set machine state to idle
    setMachineState(MACHINE_IDLE);

    // Initialize the PnP cycle switch debouncer
    pnpCycleSwitch.attach(PNP_CYCLE_SWITCH_PIN, INPUT_PULLDOWN);
    pnpCycleSwitch.interval(20); // 20ms debounce time

    Serial.println("Idle state active. Press PnP cycle switch to enter PnP mode."); 
}

void IdleState::update() {
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
    
    // Other idle state checks can go here (if any)
}

void IdleState::exit() {
    Serial.println("Exiting Idle State");
    // No specific cleanup needed for the switch here, as Bounce2 doesn't require explicit detach typically
} 