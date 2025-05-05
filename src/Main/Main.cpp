#include <Arduino.h>
#include "Setup.h"
#include "utils/machine_state.h"
#include <ArduinoOTA.h>
#include <WebSocketsServer.h>
#include "states/StateMachine.h"
#include "motors/ServoMotor.h"
#include "storage/Persistence.h"

// Include headers for functions called in loop
#include "web/Web_Dashboard_Commands.h" // For runDashboardServer()
// Add other headers as needed

extern WebSocketsServer webSocket;
extern bool webSocketServerStarted;

// Global variables
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
const int servoPin = 21;

ServoMotor myServo(servoPin);

// State machine
extern StateMachine* stateMachine;

//* ************************************************************************
//* ***************************** MAIN *******************************
//* ************************************************************************

void setup() {
  // Initialize state machine *before* system initialization, so it's available
  stateMachine = new StateMachine();

  initializeSystem();
  
  // Load the initial angle for Side 1 (or use a default)
  persistence.beginTransaction(true); // Read-only transaction
  int initialServoAngle = persistence.loadInt(SERVO_ANGLE_SIDE1_KEY, 90); // Default to 90 degrees
  persistence.endTransaction();
  myServo.init(initialServoAngle); // Initialize servo with loaded/default angle

  // Any setup code that *must* run after initializeSystem()
  Serial.println("Setup complete. Entering main loop...");
}

void loop() {
  // Handle OTA updates
  ArduinoOTA.handle();
  
  // Update machine state
  updateMachineState();
  
  // Update state machine
  if (stateMachine) {
    stateMachine->update();
  }
  
  //! Handle web server and WebSocket communication
  runDashboardServer(); // Handles incoming client connections and WebSocket messages
  
  // Add calls to other main loop functions here
  // For example, state machine updates, periodic checks, etc.
  
  delay(1); // Small delay to prevent tight loop, adjust as necessary
}
