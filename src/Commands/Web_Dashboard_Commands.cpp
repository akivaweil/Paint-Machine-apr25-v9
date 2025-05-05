#include "web/html_content.h"
#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <WiFiServer.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include "hardware/paintGun_Functions.h" // Added to access sendWebStatus
#include "motors/PaintingSides.h" // Add the new header for painting patterns
#include "storage/PaintingSettings.h" // Corrected path
#include "system/StateMachine.h" // Include StateMachine for state transitions
#include "states/ManualMoveState.h"
#include "storage/Persistence.h" // Corrected path (was persistence/persistence.h)
#include "motors/XYZ_Movements.h" // Need for moveToZ
#include "motors/ServoMotor.h" // Need for servo control
#include "motors/stepper_globals.h" // Need for stepperX, stepperY_Left etc.
#include "utils/settings.h" // Need for DEFAULT_Z_SPEED
#include <FastAccelStepper.h> // Include the full library header
#include "web/Web_Dashboard_Commands.h" // Corrected Path to header
#include <ArduinoJson.h>
#include "config.h" // Assuming this is directly under include/
#include "states/IdleState.h" // Include IdleState for comparison

//* ************************************************************************
//* ************************* WEB DASHBOARD ***************************
//* ************************************************************************

// Create a simple WiFi server on port 80
// WiFiServer dashboardServer(80); // MOVED TO SETUP.CPP
extern WiFiServer dashboardServer;
WiFiClient dashboardClient;

// Create WebSocket server on port 81
// WebSocketsServer webSocket = WebSocketsServer(81); // MOVED TO SETUP.CPP
extern WebSocketsServer webSocket;

// Flag to track WebSocket server status
// bool webSocketServerStarted = false; // MOVED TO SETUP.CPP
extern bool webSocketServerStarted; // Use global flag from Setup.cpp

// Reference to the global state machine instance (assuming it's defined in Setup.cpp or main.cpp)
extern StateMachine* stateMachine;

// Reference to the global servo motor instance
extern ServoMotor myServo;

// Declarations for functions now that Commands.h is removed
void processWebCommand(WebSocketsServer* webSocket, uint8_t num, String command);

// Remove the placeholder implementations
// Declarations for painting functions now that PaintingSides.h is removed
// void paintLeftPattern() {
//     Serial.println("Painting Left Pattern...");
//     // Implementation here
// }

// void paintRightPattern() {
//     Serial.println("Painting Right Pattern...");
//     // Implementation here  
// }

// void paintFrontPattern() {
//     Serial.println("Painting Front Pattern...");
//     // Implementation here
// }

// void paintBackPattern() {
//     Serial.println("Painting Back Pattern...");
//     // Implementation here
// }

// void paintAllSides() {
//     Serial.println("Painting All Sides...");
//     paintLeftPattern();
//     paintRightPattern();
//     paintFrontPattern();
//     paintBackPattern();
// }

// Response HTML after painting
const char* response_html = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Paint Machine Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="refresh" content="5;url=/" />
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 20px;
      text-align: center;
      background-color: #f0f0f0;
    }
    .message {
      margin-top: 20px;
      padding: 15px;
      background-color: #fff;
      border-radius: 8px;
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
    }
    .home-link {
      margin-top: 20px;
      display: inline-block;
      color: #4CAF50;
    }
  </style>
</head>
<body>
  <div class="message">
    %MESSAGE%
  </div>
  <a href="/" class="home-link">Back to Dashboard</a>
  <p>Redirecting in 5 seconds...</p>
</body>
</html>
)rawliteral";

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WS] Client #%u disconnected\n", num);
      break;
    
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[WS] Client #%u connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        
        // --- Send current state to newly connected client --- 
        if (stateMachine && stateMachine->getCurrentState()) {
            String stateMessage = "STATE:";
            stateMessage += stateMachine->getCurrentState()->getName();
            webSocket.sendTXT(num, stateMessage);
            Serial.print("Sent current state to client #");
            Serial.print(num);
            Serial.print(": ");
            Serial.println(stateMessage);
        } else {
            Serial.println("[WS] Could not send initial state: StateMachine or current state is null.");
            webSocket.sendTXT(num, "STATE:UNKNOWN"); // Send a default
        }
        // ------------------------------------------------------
      }
      break;
    
    case WStype_TEXT:
      {
        String command = String((char*)payload);
        processWebCommand(&webSocket, num, command); 
      }
      break;
      
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

// Implementation of processWebCommand
void processWebCommand(WebSocketsServer* webSocket, uint8_t num, String command) {
    Serial.print("Received command from client ");
    Serial.print(num);
    Serial.print(": ");
    Serial.println(command);

    // Declare variables used in command parsing and responses
    String baseCommand;
    String valueStr;
    float value1 = 0; 
    String message; // Declare message string here for broader scope
    
    int colonIndex = command.indexOf(':');
    
    if (colonIndex == -1) {
        baseCommand = command;
    } else {
        baseCommand = command.substring(0, colonIndex);
        valueStr = command.substring(colonIndex + 1);
        value1 = valueStr.toFloat();
    }
    baseCommand.toUpperCase();

    // --- STATE MACHINE CHECK --- 
    if (stateMachine && stateMachine->getCurrentState() != stateMachine->getIdleState() && 
        (baseCommand == "HOME_ALL" || 
         baseCommand == "START_PNP" ||
         baseCommand == "PAINT_SIDE_1" || 
         baseCommand == "PAINT_SIDE_4" || 
         baseCommand == "PAINT_SIDE_2" || 
         baseCommand == "PAINT_SIDE_3" || 
         baseCommand == "PAINT_ALL_SIDES")) { 
        Serial.print("Command ");
        Serial.print(baseCommand);
        Serial.println(" rejected. Machine must be in IDLE state.");
        webSocket->sendTXT(num, "CMD_ERROR: Machine not in IDLE state.");
        return;
    }

    // --- COMMAND PROCESSING ---
    if (baseCommand == "STATUS") {
        // Send back current status (e.g., state, positions)
        // sendWebStatus(webSocket, "STATUS_UPDATE"); // Call with a message or update internal logic
                                                   // Assuming sendWebStatus broadcasts or uses webSocket internally
                                                   // If it MUST send to 'num', sendWebStatus needs modification.
    }
    else if (baseCommand == "HOME_ALL") {
        // Trigger homing state
        if (stateMachine) {
            stateMachine->changeState(stateMachine->getHomingState());
            webSocket->sendTXT(num, "CMD_ACK: Homing sequence initiated.");
            
            // Set the home command received flag to interrupt any ongoing painting
            // homeCommandReceived = true; // REMOVED - This caused double homing when idle
        } else {
             webSocket->sendTXT(num, "CMD_ERROR: StateMachine not available.");
        }
    }
    else if (baseCommand == "START_PNP") { // Changed command name
        // // Trigger Pick and Place mode - OLD WAY
        // Serial.println("Starting PnP Setup...");
        // setup_pnp(); // OLD FUNCTION CALL - REMOVE
        // webSocket->sendTXT(num, "CMD_ACK: PnP Setup Complete");
        
        // Trigger PnP state via StateMachine - NEW WAY
        Serial.println("Transitioning to PnP State via web command...");
        if (stateMachine) {
            stateMachine->changeState(stateMachine->getPnpState());
            webSocket->sendTXT(num, "CMD_ACK: PnP State initiated.");
        } else {
            webSocket->sendTXT(num, "CMD_ERROR: StateMachine not available.");
        }
    }
    else if (baseCommand == "PAINT_GUN_ON") {
        // Turn on paint gun
        extern void paintGun_ON();
        paintGun_ON();
        webSocket->sendTXT(num, "CMD_ACK: Paint Gun ON");
    }
    else if (baseCommand == "PAINT_GUN_OFF") {
        // Turn off paint gun
        extern void paintGun_OFF();
        paintGun_OFF();
        webSocket->sendTXT(num, "CMD_ACK: Paint Gun OFF");
    }
    else if (baseCommand == "PAINT_SIDE_1") {
        Serial.println("Painting side 1...");
        paintSide1Pattern();
        if (stateMachine) stateMachine->changeState(stateMachine->getHomingState()); // Home after painting
    }
    else if (baseCommand == "PAINT_SIDE_2") {
        Serial.println("Painting side 2...");
        paintSide2Pattern();
        if (stateMachine) stateMachine->changeState(stateMachine->getHomingState()); // Home after painting
    }
    else if (baseCommand == "PAINT_SIDE_3") {
        Serial.println("Painting side 3...");
        paintSide3Pattern();
        if (stateMachine) stateMachine->changeState(stateMachine->getHomingState()); // Home after painting
    }
    else if (baseCommand == "PAINT_SIDE_4") {
        Serial.println("Painting side 4...");
        paintSide4Pattern();
        if (stateMachine) stateMachine->changeState(stateMachine->getHomingState()); // Home after painting
    }
    else if (baseCommand == "PAINT_ALL_SIDES") {
        Serial.println("Transitioning to Painting State (All Sides) via web command...");
        // Assuming PaintingState handles the specific pattern internally or via parameters
        if (stateMachine) {
            // You might need a way to tell PaintingState *which* pattern to run.
            // This could be done by setting a flag before changing state, 
            // or by passing data to the state's enter() method if possible.
            // For now, just transitioning:
            stateMachine->changeState(stateMachine->getPaintingState()); 
            webSocket->sendTXT(num, "CMD_ACK: Painting (All Sides) initiated.");
        } else {
            webSocket->sendTXT(num, "CMD_ERROR: StateMachine not available.");
        }
    }
    else if (baseCommand == "CLEAN_GUN") {
        // Enter cleaning state
        Serial.println("Entering cleaning state...");
        // Set machine state directly
        // extern void setMachineState(int state); // No need to set directly, state machine handles it
        // setMachineState(MACHINE_CLEANING);
        if (stateMachine) {
            stateMachine->changeState(stateMachine->getCleaningState());
            webSocket->sendTXT(num, "CMD_ACK: Entering Cleaning Mode");
        } else {
            webSocket->sendTXT(num, "CMD_ERROR: StateMachine not available.");
        }
    }
    else if (baseCommand == "ENTER_PICKPLACE") {
        // Enter pick and place mode via web command
        // Serial.println("Websocket: ENTER_PICKPLACE command received. Initiating PnP setup...");
        // 
        // // Call the procedural PnP setup function
        // extern void setup_pnp(); // Ensure setup_pnp is declared/included
        // setup_pnp();
        // 
        // // Set the overall machine state indicator
        // extern void setMachineState(int state);
        // setMachineState(MACHINE_PNP); 
        // 
        // // Send confirmation back to the web client
        // webSocket->sendTXT(num, "CMD_ACK: PnP Mode Initiated. Moving to standby.");

        // NEW WAY: Transition using StateMachine
        Serial.println("Websocket: ENTER_PICKPLACE command received. Transitioning to PnPState...");
        if (stateMachine) {
            stateMachine->changeState(stateMachine->getPnpState());
            webSocket->sendTXT(num, "CMD_ACK: PnP State initiated.");
        } else {
            webSocket->sendTXT(num, "CMD_ERROR: StateMachine not available.");
        }

        // Note: The actual PnP cycling will be handled by the PnPState update() method.
    }
    else if (baseCommand == "HOME") {
        // Home all axes
        Serial.println("Homing all axes...");
        // This is handled directly in webSocketEvent for immediate response
        // But we also process it here for when command comes through this function
        if (stateMachine) {
            stateMachine->changeState(stateMachine->getHomingState());
            
            // Set the home command received flag to interrupt any ongoing painting
            // homeCommandReceived = true; // REMOVED - This caused double homing when idle
        } else {
            Serial.println("ERROR: StateMachine not initialized, cannot initiate homing!");
        }
    }
    else if (baseCommand == "MOVE_Z_PREVIEW") {
        float z_pos_inch = value1;
        long z_pos_steps = (long)(z_pos_inch * STEPS_PER_INCH_XYZ);
        // Compare with StateMachine state
        if (stateMachine && (stateMachine->getCurrentState() == stateMachine->getIdleState() || stateMachine->getCurrentState() == stateMachine->getPnpState())) { 
            Serial.printf("Preview move Z to: %.2f inches (%ld steps)\n", z_pos_inch, z_pos_steps);
            // Get current X and Y to maintain position
            long currentX = stepperX->getCurrentPosition();
            long currentY = stepperY_Left->getCurrentPosition(); // Assuming Left/Right are synced
            moveToXYZ(currentX, 1, currentY, 1, z_pos_steps, DEFAULT_Z_SPEED); // Use DEFAULT_Z_SPEED, wait for completion is implicit
        } else {
            Serial.println("Preview move ignored: Machine not idle or in PnP mode.");
            webSocket->broadcastTXT("STATUS:Preview move ignored: Machine not idle or in PnP mode.");
        }
    }
    else if (baseCommand == "MOVE_SERVO_PREVIEW") {
        int angle = (int)value1;
        // Compare with StateMachine state
        if (stateMachine && (stateMachine->getCurrentState() == stateMachine->getIdleState() || stateMachine->getCurrentState() == stateMachine->getPnpState())) { 
             if (angle >= 0 && angle <= 180) {
                 Serial.printf("Preview move Servo to: %d\n", angle);
                 myServo.setAngle(angle);
             } else {
                 Serial.println("Invalid servo angle received for preview.");
                 webSocket->broadcastTXT("STATUS:Invalid servo angle received for preview.");
             }
        } else {
             Serial.println("Preview move ignored: Machine not idle or in PnP mode.");
             webSocket->broadcastTXT("STATUS:Preview move ignored: Machine not idle or in PnP mode.");
        }
    }
    else if (baseCommand == "GET_STATUS") {
        // REMOVED - State updates are handled by StateMachine broadcasts
        Serial.println("GET_STATUS command received - Handler removed (redundant).");
        webSocket->sendTXT(num, "CMD_NOTE: GET_STATUS is redundant; state is pushed automatically.");
    }
    else if (baseCommand == "GET_PATTERN_SETTINGS") {
        // Load and send existing pattern settings using persistence
        // persistence.begin(); // REMOVED - Not needed for load operations
        String settingsMsg = "PATTERN_SETTINGS:";
        settingsMsg += "paintSpeed=";
        settingsMsg += String(persistence.loadFloat(PAINT_SPEED_KEY, 10.0)); // Default 10.0
        settingsMsg += ",edgeOffset=";
        settingsMsg += String(persistence.loadFloat(EDGE_OFFSET_KEY, 0.5)); // Default 0.5
        settingsMsg += ",zClearance=";
        settingsMsg += String(persistence.loadFloat(Z_CLEARANCE_KEY, 1.0)); // Default 1.0
        settingsMsg += ",xOverlap=";
        settingsMsg += String(persistence.loadFloat(X_OVERLAP_KEY, 0.2)); // Default 0.2
        // persistence.end();
        webSocket->broadcastTXT(settingsMsg);
        Serial.println("Sent pattern settings: " + settingsMsg);
    }
    else if (baseCommand == "GET_SERVO_ANGLES") { // Handle request for servo angles
        // persistence.begin(); // REMOVED - Not needed for load operations
        String anglesMsg = "SERVO_ANGLES:";
        anglesMsg += "side1="; // Changed from top
        anglesMsg += String(paintingSettings.getSide1RotationAngle()); // NEW WAY
        anglesMsg += ",side3="; // Changed from bottom
        anglesMsg += String(paintingSettings.getSide3RotationAngle()); // NEW WAY
        anglesMsg += ",side4="; // Changed from left
        anglesMsg += String(paintingSettings.getSide4RotationAngle()); // NEW WAY
        anglesMsg += ",side2="; // Changed from right
        anglesMsg += String(paintingSettings.getSide2RotationAngle()); // NEW WAY
        // persistence.end(); // Keep open if other operations might follow quickly
        webSocket->broadcastTXT(anglesMsg);
        Serial.println("Sent servo angles: " + anglesMsg);
    }
    else if (baseCommand == "SET_PAINT_SPEED") {
         persistence.beginTransaction(false);
         persistence.saveFloat(PAINT_SPEED_KEY, value1);
         persistence.endTransaction();
         Serial.println("Saved Paint Speed: " + String(value1));
    }
    else if (baseCommand == "SET_EDGE_OFFSET") {
        persistence.beginTransaction(false);
        persistence.saveFloat(EDGE_OFFSET_KEY, value1);
        persistence.endTransaction();
        Serial.println("Saved Edge Offset: " + String(value1));
    }
    else if (baseCommand == "SET_Z_CLEARANCE") {
        persistence.beginTransaction(false);
        persistence.saveFloat(Z_CLEARANCE_KEY, value1);
        persistence.endTransaction();
        Serial.println("Saved Z Clearance: " + String(value1));
    }
    else if (baseCommand == "SET_X_OVERLAP") {
        persistence.beginTransaction(false);
        persistence.saveFloat(X_OVERLAP_KEY, value1);
        persistence.endTransaction();
        Serial.println("Saved X Overlap: " + String(value1));
    }
    else if (baseCommand == "SET_SERVO_ANGLE_SIDE1") {
        int angle = valueStr.toInt();
        paintingSettings.setServoAngleSide1(angle); // Update in memory
        paintingSettings.saveSettings(); // Save all settings
        Serial.print("Servo Angle Side 1 set to (and saved): "); // Added debug
        Serial.println(angle);
        webSocket->sendTXT(num, "CMD_ACK: Servo Angle Side 1 set and saved");
    }
    else if (baseCommand == "SET_SERVO_ANGLE_SIDE2") {
        int angle = valueStr.toInt();
        paintingSettings.setServoAngleSide2(angle); // Update in memory
        paintingSettings.saveSettings(); // Save all settings
        Serial.print("Servo Angle Side 2 set to (and saved): "); // Added debug
        Serial.println(angle);
        webSocket->sendTXT(num, "CMD_ACK: Servo Angle Side 2 set and saved");
    }
    else if (baseCommand == "SET_SERVO_ANGLE_SIDE3") {
        int angle = valueStr.toInt();
        paintingSettings.setServoAngleSide3(angle); // Update in memory
        paintingSettings.saveSettings(); // Save all settings
        Serial.print("Servo Angle Side 3 set to (and saved): "); // Added debug
        Serial.println(angle);
        webSocket->sendTXT(num, "CMD_ACK: Servo Angle Side 3 set and saved");
    }
    else if (baseCommand == "SET_SERVO_ANGLE_SIDE4") {
        int angle = valueStr.toInt();
        paintingSettings.setServoAngleSide4(angle); // Update in memory
        paintingSettings.saveSettings(); // Save all settings
        Serial.print("Servo Angle Side 4 set to (and saved): "); // Added debug
        Serial.println(angle);
        webSocket->sendTXT(num, "CMD_ACK: Servo Angle Side 4 set and saved");
    }
    else if (baseCommand == "SAVE_PAINT_SETTINGS") {
        // Save current settings to NVS
        persistence.beginTransaction(false); // Start write transaction
        paintingSettings.saveSettings(); // Save all settings managed by PaintingSettings
        persistence.endTransaction(); // End write transaction
        Serial.println("Painting settings saved to NVS via SAVE_PAINT_SETTINGS command.");

        // Send confirmation message to client
        // String message = "STATUS:Settings saved successfully"; // message is already declared
        message = "STATUS:Settings saved successfully"; 
        webSocket->broadcastTXT(message);
    }
    else if (baseCommand == "RESET_PAINT_SETTINGS") {
        // Reset painting settings to defaults
        paintingSettings.resetToDefaults();
        paintingSettings.saveSettings(); // Save defaults immediately
        webSocket->broadcastTXT("Painting settings reset to defaults");
        Serial.println("Painting settings reset to defaults");
    }
    else if (baseCommand == "SET_PAINTING_OFFSET_X") { 
        float value = value1;
        paintingSettings.setPaintingOffsetX(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Painting Offset X set to (in memory): ");
        Serial.println(paintingSettings.getPaintingOffsetX(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_PAINTING_OFFSET_Y") { 
        float value = value1;
        paintingSettings.setPaintingOffsetY(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Painting Offset Y set to (in memory): ");
        Serial.println(paintingSettings.getPaintingOffsetY(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE1ZHEIGHT") {
        float value = value1;
        paintingSettings.setSide1ZHeight(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 1 Z Height set to (in memory): ");
        Serial.println(paintingSettings.getSide1ZHeight(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE2ZHEIGHT") {
        float value = value1;
        paintingSettings.setSide2ZHeight(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 2 Z Height set to (in memory): ");
        Serial.println(paintingSettings.getSide2ZHeight(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE3ZHEIGHT") {
        float value = value1;
        paintingSettings.setSide3ZHeight(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 3 Z Height set to (in memory): ");
        Serial.println(paintingSettings.getSide3ZHeight(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE4ZHEIGHT") {
        float value = value1;
        paintingSettings.setSide4ZHeight(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 4 Z Height set to (in memory): ");
        Serial.println(paintingSettings.getSide4ZHeight(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE1SIDEZHEIGHT") {
        float value = value1;
        paintingSettings.setSide1SideZHeight(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 1 Side Z Height set to (in memory): ");
        Serial.println(paintingSettings.getSide1SideZHeight(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE2SIDEZHEIGHT") {
        float value = value1;
        paintingSettings.setSide2SideZHeight(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 2 Side Z Height set to (in memory): ");
        Serial.println(paintingSettings.getSide2SideZHeight(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE3SIDEZHEIGHT") {
        float value = value1;
        paintingSettings.setSide3SideZHeight(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 3 Side Z Height set to (in memory): ");
        Serial.println(paintingSettings.getSide3SideZHeight(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE4SIDEZHEIGHT") {
        float value = value1;
        paintingSettings.setSide4SideZHeight(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 4 Side Z Height set to (in memory): ");
        Serial.println(paintingSettings.getSide4SideZHeight(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE1SWEEPY") {
        float value = value1;
        paintingSettings.setSide1SweepY(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 1 Sweep Y set to (in memory): ");
        Serial.println(paintingSettings.getSide1SweepY(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE1SHIFTX") {
        float value = value1;
        paintingSettings.setSide1ShiftX(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 1 Shift X set to (in memory): ");
        Serial.println(paintingSettings.getSide1ShiftX(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE2SWEEPY") {
        float value = value1;
        paintingSettings.setSide2SweepY(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 2 Sweep Y set to (in memory): ");
        Serial.println(paintingSettings.getSide2SweepY(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE2SHIFTX") {
        float value = value1;
        paintingSettings.setSide2ShiftX(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 2 Shift X set to (in memory): ");
        Serial.println(paintingSettings.getSide2ShiftX(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE3SWEEPY") {
        float value = value1;
        paintingSettings.setSide3SweepY(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 3 Sweep Y set to (in memory): ");
        Serial.println(paintingSettings.getSide3SweepY(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE3SHIFTX") {
        float value = value1;
        paintingSettings.setSide3ShiftX(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 3 Shift X set to (in memory): ");
        Serial.println(paintingSettings.getSide3ShiftX(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE4SWEEPY") {
        float value = value1;
        paintingSettings.setSide4SweepY(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 4 Sweep Y set to (in memory): ");
        Serial.println(paintingSettings.getSide4SweepY(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE4SHIFTX") {
        float value = value1;
        paintingSettings.setSide4ShiftX(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 4 Shift X set to (in memory): ");
        Serial.println(paintingSettings.getSide4ShiftX(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE1_ROTATION") {
        int value = (int)value1;
        paintingSettings.setSide1RotationAngle(value);
        Serial.print("Side 1 Rotation Angle set to (in memory): ");
        Serial.println(paintingSettings.getSide1RotationAngle());
        paintingSettings.saveSettings(); // CORRECT - Keep save here for actual rotation setting
    }
    else if (baseCommand == "SET_SIDE2_ROTATION") {
        int value = (int)value1;
        paintingSettings.setSide2RotationAngle(value);
        Serial.print("Side 2 Rotation Angle set to (in memory): ");
        Serial.println(paintingSettings.getSide2RotationAngle());
        paintingSettings.saveSettings(); // CORRECT - Keep save here for actual rotation setting
    }
    else if (baseCommand == "SET_SIDE3_ROTATION") {
        int value = (int)value1;
        paintingSettings.setSide3RotationAngle(value);
        Serial.print("Side 3 Rotation Angle set to (in memory): ");
        Serial.println(paintingSettings.getSide3RotationAngle());
        paintingSettings.saveSettings(); // CORRECT - Keep save here for actual rotation setting
    }
    else if (baseCommand == "SET_SIDE4_ROTATION") {
        int value = (int)value1;
        paintingSettings.setSide4RotationAngle(value);
        Serial.print("Side 4 Rotation Angle set to (in memory): ");
        Serial.println(paintingSettings.getSide4RotationAngle());
        paintingSettings.saveSettings(); // CORRECT - Keep save here for actual rotation setting
    }
    else if (baseCommand == "SET_SIDE1PAINTINGXSPEED") {
        int value = (int)value1;
        paintingSettings.setSide1PaintingXSpeed(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 1 Painting X Speed set to (in memory): ");
        Serial.println(paintingSettings.getSide1PaintingXSpeed());
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE1PAINTINGYSPEED") {
        int value = (int)value1;
        paintingSettings.setSide1PaintingYSpeed(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 1 Painting Y Speed set to (in memory): ");
        Serial.println(paintingSettings.getSide1PaintingYSpeed());
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE2PAINTINGXSPEED") {
        int value = (int)value1;
        paintingSettings.setSide2PaintingXSpeed(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 2 Painting X Speed set to (in memory): ");
        Serial.println(paintingSettings.getSide2PaintingXSpeed());
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE2PAINTINGYSPEED") {
        int value = (int)value1;
        paintingSettings.setSide2PaintingYSpeed(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 2 Painting Y Speed set to (in memory): ");
        Serial.println(paintingSettings.getSide2PaintingYSpeed());
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE3PAINTINGXSPEED") {
        int value = (int)value1;
        paintingSettings.setSide3PaintingXSpeed(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 3 Painting X Speed set to (in memory): ");
        Serial.println(paintingSettings.getSide3PaintingXSpeed());
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE3PAINTINGYSPEED") {
        int value = (int)value1;
        paintingSettings.setSide3PaintingYSpeed(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 3 Painting Y Speed set to (in memory): ");
        Serial.println(paintingSettings.getSide3PaintingYSpeed());
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE4PAINTINGXSPEED") {
        int value = (int)value1;
        paintingSettings.setSide4PaintingXSpeed(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 4 Painting X Speed set to (in memory): ");
        Serial.println(paintingSettings.getSide4PaintingXSpeed());
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE4PAINTINGYSPEED") {
        int value = (int)value1;
        paintingSettings.setSide4PaintingYSpeed(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 4 Painting Y Speed set to (in memory): ");
        Serial.println(paintingSettings.getSide4PaintingYSpeed());
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE1STARTX") {
        float value = value1;
        paintingSettings.setSide1StartX(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 1 Start X set to (in memory): ");
        Serial.println(paintingSettings.getSide1StartX(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE1STARTY") {
        float value = value1;
        paintingSettings.setSide1StartY(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 1 Start Y set to (in memory): ");
        Serial.println(paintingSettings.getSide1StartY(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE2STARTX") {
        float value = value1;
        paintingSettings.setSide2StartX(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 2 Start X set to (in memory): ");
        Serial.println(paintingSettings.getSide2StartX(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE2STARTY") {
        float value = value1;
        paintingSettings.setSide2StartY(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 2 Start Y set to (in memory): ");
        Serial.println(paintingSettings.getSide2StartY(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE3STARTX") {
        float value = value1;
        paintingSettings.setSide3StartX(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 3 Start X set to (in memory): ");
        Serial.println(paintingSettings.getSide3StartX(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE3STARTY") {
        float value = value1;
        paintingSettings.setSide3StartY(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 3 Start Y set to (in memory): ");
        Serial.println(paintingSettings.getSide3StartY(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE4STARTX") {
        float value = value1;
        paintingSettings.setSide4StartX(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 4 Start X set to (in memory): ");
        Serial.println(paintingSettings.getSide4StartX(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_SIDE4STARTY") {
        float value = value1;
        paintingSettings.setSide4StartY(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Side 4 Start Y set to (in memory): ");
        Serial.println(paintingSettings.getSide4StartY(), 2);
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "SET_POSTPRINTPAUSE") { 
        int value = (int)value1;
        paintingSettings.setPostPrintPause(value);
        // paintingSettings.saveSettings(); // Remove internal save
        Serial.print("Post-Print Pause set to (in memory): ");
        Serial.println(paintingSettings.getPostPrintPause());
        paintingSettings.saveSettings(); // Save after setting
    }
    else if (baseCommand == "GET_PAINT_SETTINGS") {
        // Send all current painting settings to the client
        Serial.println("Sending current painting settings to client");
        
        // Paint Gun Offsets
        // String message = "SETTING:paintingOffsetX:" + String(paintingSettings.getPaintingOffsetX(), 2); // message declared above
        message = "SETTING:paintingOffsetX:" + String(paintingSettings.getPaintingOffsetX(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:paintingOffsetY:" + String(paintingSettings.getPaintingOffsetY(), 2);
        webSocket->broadcastTXT(message);
        
        // Z Heights (Order: 1, 2, 3, 4)
        message = "SETTING:side1ZHeight:" + String(paintingSettings.getSide1ZHeight(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side2ZHeight:" + String(paintingSettings.getSide2ZHeight(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side3ZHeight:" + String(paintingSettings.getSide3ZHeight(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side4ZHeight:" + String(paintingSettings.getSide4ZHeight(), 2);
        webSocket->broadcastTXT(message);
        
        // Side Z Heights (Order: 1, 2, 3, 4)
        message = "SETTING:side1SideZHeight:" + String(paintingSettings.getSide1SideZHeight(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side2SideZHeight:" + String(paintingSettings.getSide2SideZHeight(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side3SideZHeight:" + String(paintingSettings.getSide3SideZHeight(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side4SideZHeight:" + String(paintingSettings.getSide4SideZHeight(), 2);
        webSocket->broadcastTXT(message);
        
        // Rotation Angles (Order: 1, 2, 3, 4)
        message = "SETTING:side1RotationAngle:" + String(paintingSettings.getSide1RotationAngle());
        webSocket->broadcastTXT(message);
        message = "SETTING:side2RotationAngle:" + String(paintingSettings.getSide2RotationAngle());
        webSocket->broadcastTXT(message);
        message = "SETTING:side3RotationAngle:" + String(paintingSettings.getSide3RotationAngle());
        webSocket->broadcastTXT(message);
        message = "SETTING:side4RotationAngle:" + String(paintingSettings.getSide4RotationAngle());
        webSocket->broadcastTXT(message);
        
        // Painting Speeds (Order: 1, 2, 3, 4)
        message = "SETTING:side1PaintingXSpeed:" + String(paintingSettings.getSide1PaintingXSpeed());
        webSocket->broadcastTXT(message);
        message = "SETTING:side1PaintingYSpeed:" + String(paintingSettings.getSide1PaintingYSpeed());
        webSocket->broadcastTXT(message);
        message = "SETTING:side2PaintingXSpeed:" + String(paintingSettings.getSide2PaintingXSpeed());
        webSocket->broadcastTXT(message);
        message = "SETTING:side2PaintingYSpeed:" + String(paintingSettings.getSide2PaintingYSpeed());
        webSocket->broadcastTXT(message);
        message = "SETTING:side3PaintingXSpeed:" + String(paintingSettings.getSide3PaintingXSpeed());
        webSocket->broadcastTXT(message);
        message = "SETTING:side3PaintingYSpeed:" + String(paintingSettings.getSide3PaintingYSpeed());
        webSocket->broadcastTXT(message);
        message = "SETTING:side4PaintingXSpeed:" + String(paintingSettings.getSide4PaintingXSpeed());
        webSocket->broadcastTXT(message);
        message = "SETTING:side4PaintingYSpeed:" + String(paintingSettings.getSide4PaintingYSpeed());
        webSocket->broadcastTXT(message);
        
        // Pattern Start Positions (Order: 1, 2, 3, 4)
        message = "SETTING:side1StartX:" + String(paintingSettings.getSide1StartX(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side1StartY:" + String(paintingSettings.getSide1StartY(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side2StartX:" + String(paintingSettings.getSide2StartX(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side2StartY:" + String(paintingSettings.getSide2StartY(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side3StartX:" + String(paintingSettings.getSide3StartX(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side3StartY:" + String(paintingSettings.getSide3StartY(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side4StartX:" + String(paintingSettings.getSide4StartX(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side4StartY:" + String(paintingSettings.getSide4StartY(), 2);
        webSocket->broadcastTXT(message);
        
        // Pattern Dimensions (Order: 1, 2, 3, 4)
        message = "SETTING:side1SweepY:" + String(paintingSettings.getSide1SweepY(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side1ShiftX:" + String(paintingSettings.getSide1ShiftX(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side2SweepY:" + String(paintingSettings.getSide2SweepY(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side2ShiftX:" + String(paintingSettings.getSide2ShiftX(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side3SweepY:" + String(paintingSettings.getSide3SweepY(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side3ShiftX:" + String(paintingSettings.getSide3ShiftX(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side4SweepY:" + String(paintingSettings.getSide4SweepY(), 2);
        webSocket->broadcastTXT(message);
        message = "SETTING:side4ShiftX:" + String(paintingSettings.getSide4ShiftX(), 2);
        webSocket->broadcastTXT(message);
        
        // Post-Print Pause
        message = "SETTING:postPrintPause:" + String(paintingSettings.getPostPrintPause());
        webSocket->broadcastTXT(message);
        
        // Servo Angles (Order: 1, 2, 3, 4)
        // NOTE: Originally read directly from NVS using old keys. Changed to use getters 
        // from the paintingSettings object to ensure consistency and fix persistence issue.
        message = "SETTING:servoAngleSide1:" + String(paintingSettings.getServoAngleSide1()); // Use getter
        webSocket->broadcastTXT(message);
        message = "SETTING:servoAngleSide2:" + String(paintingSettings.getServoAngleSide2()); // Use getter
        webSocket->broadcastTXT(message);
        message = "SETTING:servoAngleSide3:" + String(paintingSettings.getServoAngleSide3()); // Use getter
        webSocket->broadcastTXT(message);
        message = "SETTING:servoAngleSide4:" + String(paintingSettings.getServoAngleSide4()); // Use getter
        webSocket->broadcastTXT(message);
    }
    else if (baseCommand == "ENTER_MANUAL_MODE") {
        if (stateMachine) {
            // Only allow entering manual mode from Idle or maybe PnP?
            State* currentState = stateMachine->getCurrentState();
            if (currentState == stateMachine->getIdleState() || currentState == stateMachine->getPnpState()) { 
                Serial.println("Transitioning to Manual Move State via web command...");
                stateMachine->changeState(stateMachine->getManualMoveState());
                webSocket->sendTXT(num, "CMD_ACK: Manual Move Mode entered.");
            } else {
                Serial.print("Cannot enter Manual Mode from current state: ");
                // Serial.println(stateMachine->getStateName(currentState)); // Requires getStateName implementation
                webSocket->sendTXT(num, "CMD_ERROR: Cannot enter Manual Mode from current state.");
            }
        } else {
            webSocket->sendTXT(num, "CMD_ERROR: StateMachine not available.");
        }
    }
    else if (baseCommand == "EXIT_MANUAL_MODE") {
         if (stateMachine && stateMachine->getCurrentState() == stateMachine->getManualMoveState()) {
            Serial.println("Exiting Manual Move State via web command...");
            stateMachine->changeState(stateMachine->getIdleState()); // Return to Idle
            webSocket->sendTXT(num, "CMD_ACK: Manual Move Mode exited.");
        } else {
            webSocket->sendTXT(num, "CMD_ERROR: Not in Manual Move Mode.");
        }
    }
    else if (baseCommand == "MANUAL_MOVE_TO") {
        if (stateMachine && stateMachine->getCurrentState() == stateMachine->getManualMoveState()) {
            // Expected format: MANUAL_MOVE_TO:x,y,z,angle
            // valueStr should contain "x,y,z,angle"
            
            // Parse the comma-separated values
            long targetX, targetY, targetZ;
            int targetAngle;
            int firstComma = valueStr.indexOf(',');
            int secondComma = valueStr.indexOf(',', firstComma + 1);
            int thirdComma = valueStr.indexOf(',', secondComma + 1);

            if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
                targetX = valueStr.substring(0, firstComma).toFloat() * STEPS_PER_INCH_XYZ; // Assuming input is inches
                targetY = valueStr.substring(firstComma + 1, secondComma).toFloat() * STEPS_PER_INCH_XYZ;
                targetZ = valueStr.substring(secondComma + 1, thirdComma).toFloat() * STEPS_PER_INCH_XYZ;
                targetAngle = valueStr.substring(thirdComma + 1).toInt();

                // Cast the state pointer and call the move function
                ManualMoveState* manualState = static_cast<ManualMoveState*>(stateMachine->getManualMoveState());
                if (manualState) {
                    manualState->moveToPosition(targetX, targetY, targetZ, targetAngle);
                    webSocket->sendTXT(num, "CMD_ACK: Manual move executed.");
                } else {
                     webSocket->sendTXT(num, "CMD_ERROR: Could not get ManualMoveState object."); // Should not happen
                }
            } else {
                Serial.println("Invalid format for MANUAL_MOVE_TO. Expected: x,y,z,angle");
                webSocket->sendTXT(num, "CMD_ERROR: Invalid format for MANUAL_MOVE_TO. Expected: x,y,z,angle");
            }
        } else {
            Serial.println("MANUAL_MOVE_TO command ignored: Not in Manual Move Mode.");
            webSocket->sendTXT(num, "CMD_ERROR: Not in Manual Move Mode.");
        }
    }
    else if (baseCommand == "PNP_SAVE_POSITION") {
        // Placeholder - Add implementation if needed
        Serial.println("PNP_SAVE_POSITION command received - Not implemented");
        webSocket->sendTXT(num, "CMD_ERROR: PNP_SAVE_POSITION not implemented");
    }
    else {
        // Unknown command
        Serial.print("Unknown command received: ");
        Serial.println(command);
        webSocket->sendTXT(num, "CMD_ERROR: Unknown command");
    }
}

// Function that handles WebSocket events without executing commands
// This is used during painting to check for HOME commands
void processWebSocketEvents() {
  // Process WebSocket events to allow receiving commands
  // This is called during operations that need to be interruptible
  webSocket.loop();
  
  // Give some time for any WebSocket messages to be processed
  // This ensures the webSocketEvent handler can set flags like homeCommandReceived
  for (int i = 0; i < 10; i++) {
    webSocket.loop();
    delay(1);
  }
}

// Function to check for HOME command during painting operations
// Returns true if a home command was received
extern volatile bool homeCommandReceived; // Assume declared globally
bool checkForHomeCommand() {
  // Process any pending WebSocket events
  processWebSocketEvents();
  
  // Check if a home command was received
  if (homeCommandReceived) {
    Serial.println("HOME command received during painting - aborting");
    return true;
  }
  
  return false;
}

// Function to check and restart WebSocket if needed
void ensureWebSocketRunning() {
  if (!webSocketServerStarted || WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_CONNECTED) {
      // Stop WebSocket if it was already running
      webSocket.close();
      delay(50);
      
      // Restart WebSocket
      webSocket.begin();
      webSocket.onEvent(webSocketEvent);
      webSocketServerStarted = true;
      Serial.println("WebSocket server (re)started on port 81");
    } else {
      if (webSocketServerStarted) {
        webSocket.close();
        webSocketServerStarted = false;
        Serial.println("WebSocket server stopped due to WiFi disconnection");
      }
    }
  }
}

// Parse the HTTP request line
String parseRequestLine(String line) {
  // Extract the URL from the request line
  // Typical format: "GET /path HTTP/1.1"
  int startPos = line.indexOf(' ');
  if (startPos < 0) return "";
  
  int endPos = line.indexOf(' ', startPos + 1);
  if (endPos < 0) return "";
  
  return line.substring(startPos + 1, endPos);
}

// Extract value of URL parameter
String getParameter(String url, String param) {
  int startPos = url.indexOf(param + "=");
  if (startPos < 0) return "";
  
  startPos += param.length() + 1;
  int endPos = url.indexOf('&', startPos);
  if (endPos < 0) endPos = url.length();
  
  return url.substring(startPos, endPos);
}

// Handle HTTP request
void handleDashboardClient() {
  dashboardClient = dashboardServer.available();
  if (dashboardClient) {
    String currentLine = "";
    String request = ""; // Initialize request string
    
    while (dashboardClient.connected()) {
      if (dashboardClient.available()) {
        char c = dashboardClient.read();
        
        // Store the first line of the HTTP request
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // End of HTTP headers, send response based on the request
            
            // Parse the request line *here* 
            int firstSpace = request.indexOf(' ');
            int secondSpace = request.indexOf(' ', firstSpace + 1);
            String requestPath = "";
            if (firstSpace != -1 && secondSpace != -1) {
                requestPath = request.substring(firstSpace + 1, secondSpace);
            }
            
            if (requestPath == "/") {
              // Root page - send the dashboard
              dashboardClient.println("HTTP/1.1 200 OK");
              dashboardClient.println("Content-Type: text/html");
              dashboardClient.println("Connection: close");
              dashboardClient.println();
              dashboardClient.println(HTML_PROGMEM);
              // break; // Don't break here, let the client close or timeout
            } 
            else if (requestPath == "/settings") {
              // Settings page - send the same HTML but the client-side JS will show settings
              dashboardClient.println("HTTP/1.1 200 OK");
              dashboardClient.println("Content-Type: text/html");
              dashboardClient.println("Connection: close");
              dashboardClient.println();
              dashboardClient.println(HTML_PROGMEM);
              // break; // Headers processed, response sent, let client close
            }
            else if (requestPath.startsWith("/paint?")) { // Check for /paint with parameters
              // Paint command - Parse parameters from requestPath
              String side = "";
              int qPos = requestPath.indexOf('?');
              if (qPos != -1) {
                  String params = requestPath.substring(qPos + 1);
                  int eqPos = params.indexOf("side=");
                  if (eqPos != -1) {
                      side = params.substring(eqPos + 5);
                      int ampersandPos = side.indexOf('&'); // Remove potential extra params
                      if (ampersandPos != -1) {
                          side = side.substring(0, ampersandPos);
                      }
                  }
              }
                
              String message = "Unknown side: " + side;
              bool painted = false;
              
              // Process the paint command
              if (side == "side4") {
                message = "Painting left side...";
                painted = true;
              }
              else if (side == "side2") {
                message = "Painting right side...";
                painted = true;
              }
              else if (side == "side1") {
                message = "Painting front side...";
                painted = true;
              }
              else if (side == "side3") {
                message = "Painting back side...";
                painted = true;
              }
              else if (side == "all") {
                message = "Painting all sides...";
                painted = true; // Or handle separately if needed
              }
              
              // Send response first
              dashboardClient.println("HTTP/1.1 200 OK");
              dashboardClient.println("Content-Type: text/html");
              dashboardClient.println("Connection: close"); // Important: signal connection close
              dashboardClient.println();
              String response = String(response_html);
              response.replace("%MESSAGE%", message);
              dashboardClient.println(response);
              dashboardClient.flush(); // Ensure response is sent 
              // dashboardClient.stop();  // Let the client close the connection after receiving
              // Delay slightly to ensure data is sent before potentially long operation
              delay(50);
              
              // Now execute the paint job if a valid side was given
              if (painted) {
                  Serial.println(message); // Log the action
                  if (side == "side4") paintSide4Pattern();
                  else if (side == "side2") paintSide2Pattern();
                  else if (side == "side1") paintSide1Pattern();
                  else if (side == "side3") paintSide3Pattern();
                  else if (side == "all") { /* Call paintAllSides() or similar */ }
                  Serial.println(side + " side painting completed");
              }
              break; // Break after handling paint request completely
            }
            else {
              // Not found
              dashboardClient.println("HTTP/1.1 404 Not Found");
              dashboardClient.println("Content-Type: text/html");
              dashboardClient.println("Connection: close");
              dashboardClient.println();
              dashboardClient.println("<html><body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p><a href='/'>Back to Dashboard</a></body></html>");
              // break; // Let client close
            }
            // End of request handling, break the inner while loop
            break; 
          } else {
            // If we got a newline, check if this is the request line
            if (currentLine.startsWith("GET ") || currentLine.startsWith("POST ")) { // Handle GET or POST
              request = currentLine; // Store the request line
            }
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      } // if dashboardClient.available()
    } // while dashboardClient.connected()
    
    // Close the connection 
    dashboardClient.stop();
  } // if dashboardClient
}

void runDashboardServer() {
  // Ensure WebSocket is running if WiFi is connected
  ensureWebSocketRunning();
  
  // Handle client requests
  handleDashboardClient();
  
  // Handle WebSocket events
  webSocket.loop();
}

void stopDashboardServer() {
  // Stop the HTTP server
  dashboardServer.end();
  
  // Stop the WebSocket server
  webSocket.close();
  webSocketServerStarted = false;
  
  Serial.println("Dashboard web server and WebSocket server stopped");
}

// Global variables for WiFi, WebServer, WebSockets
// WiFiServer server(80); // Duplicate - Remove
// String header; // Likely unused - Remove

// Function Implementations
/* REMOVED - Logic moved to Setup.cpp and called via initializeWebCommunications()
void setupWebDashboard() {
    const char *ssid = "Everwood"; // Replace with your network SSID
    const char *password = "Everwood-Staff"; // Replace with your network password

    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        Serial.print(".");
    }
    Serial.println(" Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Setup mDNS
    if (!MDNS.begin("paint-machine")) { // paint-machine.local
        Serial.println("Error setting up MDNS responder!");
    } else {
        Serial.println("mDNS responder started: http://paint-machine.local");
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("ws", "tcp", 81);
    }

    // Start HTTP server
    dashboardServer.begin();
    Serial.println("HTTP server started on port 80");

    // Start WebSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    webSocketServerStarted = true;
    Serial.println("WebSocket server started on port 81");
}
*/ 