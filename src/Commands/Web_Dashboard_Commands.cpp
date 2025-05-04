#include "web/html_content.h"
#include <Arduino.h>
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
#include "system/machine_state.h"
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
        // Send initial status or welcome message if needed
        // Example: webSocket.sendTXT(num, "Welcome!");
      }
      break;
    
    case WStype_TEXT:
      // Process the command from the WebSocket
      {
        String command = String((char*)payload);
        
        // Pass the client number (num) to processWebCommand
        processWebCommand(&webSocket, num, command); 
      }
      break;
      
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      // We don't handle these types
      break;
  }
}

// Implementation of processWebCommand - updated to accept client number
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
    
    int colonIndex = command.indexOf(':'); // Find the colon
    
    if (colonIndex == -1) {
        // No colon found, treat the whole string as the command
        baseCommand = command;
    } else {
        // Colon found, split into command and value string
        baseCommand = command.substring(0, colonIndex);
        valueStr = command.substring(colonIndex + 1);
        
        // Attempt to convert the value string to a float
        value1 = valueStr.toFloat();
        // Note: toFloat() returns 0.0 if conversion fails, which might be acceptable or require error handling
    }
    baseCommand.toUpperCase(); // Standardize command

    // --- STATE MACHINE CHECK --- 
    // Prevent certain actions unless in IDLE state?
    bool isIdle = (stateMachine && stateMachine->getCurrentState() == stateMachine->getIdleState());
    if (!isIdle && 
        (baseCommand == "HOME_ALL" || 
         baseCommand == "START_PNP" || // Changed from G28
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
        sendWebStatus(webSocket, "STATUS_UPDATE"); // Call with a message or update internal logic
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
            webSocket->sendTXT(num, "CMD_ACK: PnP State initiated."); // Use same ACK as START_PNP
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
        long z_pos_steps = (long)(z_pos_inch * STEPS_PER_INCH_XYZ); // Convert inches to steps
        if (getMachineState() == MACHINE_IDLE || getMachineState() == MACHINE_PNP) { // Only allow preview moves when idle or in PnP
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
        if (getMachineState() == MACHINE_IDLE || getMachineState() == MACHINE_PNP) { // Only allow preview moves when idle or in PnP
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
        // Send current status updates to client
        Serial.println("Sending status updates to client");
        
        // Send pressure pot status
        extern bool isPressurePot_ON;
        if (isPressurePot_ON) {
            webSocket->broadcastTXT("PRESSURE_POT_STATUS:ON");
        } else {
            webSocket->broadcastTXT("PRESSURE_POT_STATUS:OFF");
        }
        
        // Send paint gun status (determine if it's on)
        extern bool isPaintGun_ON;
        if (isPaintGun_ON) {
            webSocket->broadcastTXT("PAINT_GUN_STATUS:ON");
        } else {
            webSocket->broadcastTXT("PAINT_GUN_STATUS:OFF");
        }

        // Send machine state
        String stateStr = "UNKNOWN";
        switch(getMachineState()) {
            case MACHINE_IDLE: stateStr = "Idle"; break;
            case MACHINE_HOMING: stateStr = "Homing..."; break;
            case MACHINE_PAINTING: stateStr = "Painting..."; break;
            case MACHINE_PNP: stateStr = "Pick & Place Mode"; break;
            case MACHINE_CLEANING: stateStr = "Cleaning..."; break;
            case MACHINE_ERROR: stateStr = "Error"; break;
        }
        webSocket->broadcastTXT("MACHINE_STATUS:" + stateStr);
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
        anglesMsg += String(persistence.loadInt(SERVO_ANGLE_SIDE1_KEY, 90)); // Default 90
        anglesMsg += ",side3="; // Changed from bottom
        anglesMsg += String(persistence.loadInt(SERVO_ANGLE_SIDE3_KEY, 90)); // Default 90
        anglesMsg += ",side4="; // Changed from left
        anglesMsg += String(persistence.loadInt(SERVO_ANGLE_SIDE4_KEY, 90)); // Default 90
        anglesMsg += ",side2="; // Changed from right
        anglesMsg += String(persistence.loadInt(SERVO_ANGLE_SIDE2_KEY, 90)); // Default 90
        // persistence.end(); // Keep open if other operations might follow quickly
        webSocket->broadcastTXT(anglesMsg);
        Serial.println("Sent servo angles: " + anglesMsg);
        
        // Servo Angles (Order: 1, 2, 3, 4)
        // persistence.begin(); // REMOVED - Not needed for load operations
        message = "SETTING:servoAngleSide1:" + String(persistence.loadInt(SERVO_ANGLE_SIDE1_KEY, 0));
        webSocket->broadcastTXT(message);
        message = "SETTING:servoAngleSide2:" + String(persistence.loadInt(SERVO_ANGLE_SIDE2_KEY, 0));
        webSocket->broadcastTXT(message);
        message = "SETTING:servoAngleSide3:" + String(persistence.loadInt(SERVO_ANGLE_SIDE3_KEY, 0));
        webSocket->broadcastTXT(message);
        message = "SETTING:servoAngleSide4:" + String(persistence.loadInt(SERVO_ANGLE_SIDE4_KEY, 0));
        webSocket->broadcastTXT(message);
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
    else if (baseCommand == "SET_SERVO_ANGLE_SIDE1") { // Renamed command
        int angle = (int)value1;
        if (angle >= 0 && angle <= 180) {
            persistence.beginTransaction(false);
            persistence.saveInt(SERVO_ANGLE_SIDE1_KEY, angle); // Updated key
            persistence.endTransaction();
            Serial.println("Saved Front Servo Angle: " + String(angle));
        } else {
            Serial.println("Invalid servo angle received: " + String(value1));
        }
    }
    else if (baseCommand == "SET_SERVO_ANGLE_SIDE2") { // Renamed command
        int angle = (int)value1;
        if (angle >= 0 && angle <= 180) {
            persistence.beginTransaction(false);
            persistence.saveInt(SERVO_ANGLE_SIDE2_KEY, angle); // Updated key
            persistence.endTransaction();
            Serial.println("Saved Right Servo Angle: " + String(angle));
        } else {
            Serial.println("Invalid servo angle received: " + String(value1));
        }
    }
    else if (baseCommand == "SET_SERVO_ANGLE_SIDE3") { // Renamed command
        int angle = (int)value1;
        if (angle >= 0 && angle <= 180) { // Basic validation
            persistence.beginTransaction(false);
            persistence.saveInt(SERVO_ANGLE_SIDE3_KEY, angle); // Updated key
            persistence.endTransaction();
            Serial.println("Saved Back Servo Angle: " + String(angle));
        } else {
            Serial.println("Invalid servo angle received: " + String(value1));
        }
    }
    else if (baseCommand == "SET_SERVO_ANGLE_SIDE4") { // Renamed command
        int angle = (int)value1;
        if (angle >= 0 && angle <= 180) {
            persistence.beginTransaction(false);
            persistence.saveInt(SERVO_ANGLE_SIDE4_KEY, angle); // Updated key
            persistence.endTransaction();
            Serial.println("Saved Left Servo Angle: " + String(angle));
        } else {
            Serial.println("Invalid servo angle received: " + String(value1));
        }
    }
    else if (baseCommand == "SAVE_PAINT_SETTINGS") {
        // Save current settings to NVS
        // Preferences are typically saved automatically or when end() is called.
        // No explicit action needed here as individual saves happened already.
        // persistence.begin(); // REMOVED - Let paintingSettings.saveSettings handle it if needed
        paintingSettings.saveSettings(); // Explicitly save all settings managed by PaintingSettings
        // persistence.end(); // Optionally close and commit everything now.
        Serial.println("Persistence commit triggered (if needed by Preferences lib).");

        // Send confirmation message to client
        String message = "STATUS:Settings saved successfully";
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
        paintingSettings.saveSettings(); // Save immediately after setting
        Serial.print("Painting Offset X set to: ");
        Serial.println(paintingSettings.getPaintingOffsetX(), 2);
    }
    else if (baseCommand == "SET_PAINTING_OFFSET_Y") { 
        float value = value1;
        paintingSettings.setPaintingOffsetY(value);
        paintingSettings.saveSettings(); // Save immediately after setting
        Serial.print("Painting Offset Y set to: ");
        Serial.println(paintingSettings.getPaintingOffsetY(), 2);
    }
    else if (baseCommand == "SET_SIDE1ZHEIGHT") {
        float value = value1;
        paintingSettings.setSide1ZHeight(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 1 Z Height set to: ");
        Serial.println(paintingSettings.getSide1ZHeight(), 2);
    }
    else if (baseCommand == "SET_SIDE2ZHEIGHT") {
        float value = value1;
        paintingSettings.setSide2ZHeight(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 2 Z Height set to: ");
        Serial.println(paintingSettings.getSide2ZHeight(), 2);
    }
    else if (baseCommand == "SET_SIDE3ZHEIGHT") {
        float value = value1;
        paintingSettings.setSide3ZHeight(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 3 Z Height set to: ");
        Serial.println(paintingSettings.getSide3ZHeight(), 2);
    }
    else if (baseCommand == "SET_SIDE4ZHEIGHT") {
        float value = value1;
        paintingSettings.setSide4ZHeight(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 4 Z Height set to: ");
        Serial.println(paintingSettings.getSide4ZHeight(), 2);
    }
    else if (baseCommand == "SET_SIDE1SIDEZHEIGHT") {
        float value = value1;
        paintingSettings.setSide1SideZHeight(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 1 Side Z Height set to: ");
        Serial.println(paintingSettings.getSide1SideZHeight(), 2);
    }
    else if (baseCommand == "SET_SIDE2SIDEZHEIGHT") {
        float value = value1;
        paintingSettings.setSide2SideZHeight(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 2 Side Z Height set to: ");
        Serial.println(paintingSettings.getSide2SideZHeight(), 2);
    }
    else if (baseCommand == "SET_SIDE3SIDEZHEIGHT") {
        float value = value1;
        paintingSettings.setSide3SideZHeight(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 3 Side Z Height set to: ");
        Serial.println(paintingSettings.getSide3SideZHeight(), 2);
    }
    else if (baseCommand == "SET_SIDE4SIDEZHEIGHT") {
        float value = value1;
        paintingSettings.setSide4SideZHeight(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 4 Side Z Height set to: ");
        Serial.println(paintingSettings.getSide4SideZHeight(), 2);
    }
    else if (baseCommand == "SET_SIDE1SWEEPY") {
        float value = value1;
        paintingSettings.setSide1SweepY(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 1 Sweep Y set to: ");
        Serial.println(paintingSettings.getSide1SweepY(), 2);
    }
    else if (baseCommand == "SET_SIDE1SHIFTX") {
        float value = value1;
        paintingSettings.setSide1ShiftX(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 1 Shift X set to: ");
        Serial.println(paintingSettings.getSide1ShiftX(), 2);
    }
    else if (baseCommand == "SET_SIDE2SWEEPY") {
        float value = value1;
        paintingSettings.setSide2SweepY(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 2 Sweep Y set to: ");
        Serial.println(paintingSettings.getSide2SweepY(), 2);
    }
    else if (baseCommand == "SET_SIDE2SHIFTX") {
        float value = value1;
        paintingSettings.setSide2ShiftX(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 2 Shift X set to: ");
        Serial.println(paintingSettings.getSide2ShiftX(), 2);
    }
    else if (baseCommand == "SET_SIDE3SWEEPY") {
        float value = value1;
        paintingSettings.setSide3SweepY(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 3 Sweep Y set to: ");
        Serial.println(paintingSettings.getSide3SweepY(), 2);
    }
    else if (baseCommand == "SET_SIDE3SHIFTX") {
        float value = value1;
        paintingSettings.setSide3ShiftX(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 3 Shift X set to: ");
        Serial.println(paintingSettings.getSide3ShiftX(), 2);
    }
    else if (baseCommand == "SET_SIDE4SWEEPY") {
        float value = value1;
        paintingSettings.setSide4SweepY(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 4 Sweep Y set to: ");
        Serial.println(paintingSettings.getSide4SweepY(), 2);
    }
    else if (baseCommand == "SET_SIDE4SHIFTX") {
        float value = value1;
        paintingSettings.setSide4ShiftX(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 4 Shift X set to: ");
        Serial.println(paintingSettings.getSide4ShiftX(), 2);
    }
    else if (baseCommand == "SET_SIDE1_ROTATION") {
        int value = (int)value1;
        paintingSettings.setSide1RotationAngle(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 1 Rotation Angle set to: ");
        Serial.println(paintingSettings.getSide1RotationAngle());
    }
    else if (baseCommand == "SET_SIDE2_ROTATION") {
        int value = (int)value1;
        paintingSettings.setSide2RotationAngle(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 2 Rotation Angle set to: ");
        Serial.println(paintingSettings.getSide2RotationAngle());
    }
    else if (baseCommand == "SET_SIDE3_ROTATION") {
        int value = (int)value1;
        paintingSettings.setSide3RotationAngle(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 3 Rotation Angle set to: ");
        Serial.println(paintingSettings.getSide3RotationAngle());
    }
    else if (baseCommand == "SET_SIDE4_ROTATION") {
        int value = (int)value1;
        paintingSettings.setSide4RotationAngle(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 4 Rotation Angle set to: ");
        Serial.println(paintingSettings.getSide4RotationAngle());
    }
    else if (baseCommand == "SET_SIDE1PAINTINGXSPEED") {
        int value = (int)value1;
        paintingSettings.setSide1PaintingXSpeed(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 1 Painting X Speed set to: ");
        Serial.println(paintingSettings.getSide1PaintingXSpeed());
    }
    else if (baseCommand == "SET_SIDE1PAINTINGYSPEED") {
        int value = (int)value1;
        paintingSettings.setSide1PaintingYSpeed(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 1 Painting Y Speed set to: ");
        Serial.println(paintingSettings.getSide1PaintingYSpeed());
    }
    else if (baseCommand == "SET_SIDE2PAINTINGXSPEED") {
        int value = (int)value1;
        paintingSettings.setSide2PaintingXSpeed(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 2 Painting X Speed set to: ");
        Serial.println(paintingSettings.getSide2PaintingXSpeed());
    }
    else if (baseCommand == "SET_SIDE2PAINTINGYSPEED") {
        int value = (int)value1;
        paintingSettings.setSide2PaintingYSpeed(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 2 Painting Y Speed set to: ");
        Serial.println(paintingSettings.getSide2PaintingYSpeed());
    }
    else if (baseCommand == "SET_SIDE3PAINTINGXSPEED") {
        int value = (int)value1;
        paintingSettings.setSide3PaintingXSpeed(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 3 Painting X Speed set to: ");
        Serial.println(paintingSettings.getSide3PaintingXSpeed());
    }
    else if (baseCommand == "SET_SIDE3PAINTINGYSPEED") {
        int value = (int)value1;
        paintingSettings.setSide3PaintingYSpeed(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 3 Painting Y Speed set to: ");
        Serial.println(paintingSettings.getSide3PaintingYSpeed());
    }
    else if (baseCommand == "SET_SIDE4PAINTINGXSPEED") {
        int value = (int)value1;
        paintingSettings.setSide4PaintingXSpeed(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 4 Painting X Speed set to: ");
        Serial.println(paintingSettings.getSide4PaintingXSpeed());
    }
    else if (baseCommand == "SET_SIDE4PAINTINGYSPEED") {
        int value = (int)value1;
        paintingSettings.setSide4PaintingYSpeed(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 4 Painting Y Speed set to: ");
        Serial.println(paintingSettings.getSide4PaintingYSpeed());
    }
    else if (baseCommand == "SET_SIDE1STARTX") {
        float value = value1;
        paintingSettings.setSide1StartX(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 1 Start X set to: ");
        Serial.println(paintingSettings.getSide1StartX(), 2);
    }
    else if (baseCommand == "SET_SIDE1STARTY") {
        float value = value1;
        paintingSettings.setSide1StartY(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 1 Start Y set to: ");
        Serial.println(paintingSettings.getSide1StartY(), 2);
    }
    else if (baseCommand == "SET_SIDE2STARTX") {
        float value = value1;
        paintingSettings.setSide2StartX(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 2 Start X set to: ");
        Serial.println(paintingSettings.getSide2StartX(), 2);
    }
    else if (baseCommand == "SET_SIDE2STARTY") {
        float value = value1;
        paintingSettings.setSide2StartY(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 2 Start Y set to: ");
        Serial.println(paintingSettings.getSide2StartY(), 2);
    }
    else if (baseCommand == "SET_SIDE3STARTX") {
        float value = value1;
        paintingSettings.setSide3StartX(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 3 Start X set to: ");
        Serial.println(paintingSettings.getSide3StartX(), 2);
    }
    else if (baseCommand == "SET_SIDE3STARTY") {
        float value = value1;
        paintingSettings.setSide3StartY(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 3 Start Y set to: ");
        Serial.println(paintingSettings.getSide3StartY(), 2);
    }
    else if (baseCommand == "SET_SIDE4STARTX") {
        float value = value1;
        paintingSettings.setSide4StartX(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 4 Start X set to: ");
        Serial.println(paintingSettings.getSide4StartX(), 2);
    }
    else if (baseCommand == "SET_SIDE4STARTY") {
        float value = value1;
        paintingSettings.setSide4StartY(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Side 4 Start Y set to: ");
        Serial.println(paintingSettings.getSide4StartY(), 2);
    }
    else if (baseCommand == "SET_POSTPRINTPAUSE") { 
        int value = (int)value1;
        paintingSettings.setPostPrintPause(value);
        paintingSettings.saveSettings(); // Add save
        Serial.print("Post-Print Pause set to: ");
        Serial.println(paintingSettings.getPostPrintPause());
    }
    else if (baseCommand == "GET_PAINT_SETTINGS") {
        // Send all current painting settings to the client
        Serial.println("Sending current painting settings to client");
        
        // Paint Gun Offsets
        String message = "SETTING:paintingOffsetX:" + String(paintingSettings.getPaintingOffsetX(), 2);
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
        // persistence.begin(); // REMOVED - Not needed for load operations
        message = "SETTING:servoAngleSide1:" + String(persistence.loadInt(SERVO_ANGLE_SIDE1_KEY, 0));
        webSocket->broadcastTXT(message);
        message = "SETTING:servoAngleSide2:" + String(persistence.loadInt(SERVO_ANGLE_SIDE2_KEY, 0));
        webSocket->broadcastTXT(message);
        message = "SETTING:servoAngleSide3:" + String(persistence.loadInt(SERVO_ANGLE_SIDE3_KEY, 0));
        webSocket->broadcastTXT(message);
        message = "SETTING:servoAngleSide4:" + String(persistence.loadInt(SERVO_ANGLE_SIDE4_KEY, 0));
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
                Serial.println(stateMachine->getStateName(currentState));
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
        // ... existing code ...
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
    String request = "";
    
    while (dashboardClient.connected()) {
      if (dashboardClient.available()) {
        char c = dashboardClient.read();
        
        // Store the first line of the HTTP request
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // End of HTTP headers, send response based on the request
            
            if (request.startsWith("GET / ")) {
              // Root page - send the dashboard
              dashboardClient.println("HTTP/1.1 200 OK");
              dashboardClient.println("Content-Type: text/html");
              dashboardClient.println("Connection: close");
              dashboardClient.println();
              dashboardClient.println(HTML_PROGMEM);
              break;
            } 
            else if (request.startsWith("GET /settings")) {
              // Settings page - send the same HTML but the client-side JS will show settings
              dashboardClient.println("HTTP/1.1 200 OK");
              dashboardClient.println("Content-Type: text/html");
              dashboardClient.println("Connection: close");
              dashboardClient.println();
              dashboardClient.println(HTML_PROGMEM);
              break;
            }
            else if (request.startsWith("GET /paint")) {
              // Paint command
              String side = getParameter(request, "side");
              String message = "Unknown side";
              
              // Process the paint command
              if (side == "side4") {
                message = "Painting left side...";
                dashboardClient.println("HTTP/1.1 200 OK");
                dashboardClient.println("Content-Type: text/html");
                dashboardClient.println("Connection: close");
                dashboardClient.println();
                
                String response = String(response_html);
                response.replace("%MESSAGE%", message);
                dashboardClient.println(response);
                
                dashboardClient.flush();
                dashboardClient.stop();
                
                // Now paint (after client disconnected)
                Serial.println(message);
                paintSide4Pattern();
                Serial.println("Left side painting completed");
                return;
              }
              else if (side == "side2") {
                message = "Painting right side...";
                dashboardClient.println("HTTP/1.1 200 OK");
                dashboardClient.println("Content-Type: text/html");
                dashboardClient.println("Connection: close");
                dashboardClient.println();
                
                String response = String(response_html);
                response.replace("%MESSAGE%", message);
                dashboardClient.println(response);
                
                dashboardClient.flush();
                dashboardClient.stop();
                
                // Now paint
                Serial.println(message);
                paintSide2Pattern();
                Serial.println("Right side painting completed");
                return;
              }
              else if (side == "side1") {
                message = "Painting front side...";
                dashboardClient.println("HTTP/1.1 200 OK");
                dashboardClient.println("Content-Type: text/html");
                dashboardClient.println("Connection: close");
                dashboardClient.println();
                
                String response = String(response_html);
                response.replace("%MESSAGE%", message);
                dashboardClient.println(response);
                
                dashboardClient.flush();
                dashboardClient.stop();
                
                // Now paint
                Serial.println(message);
                paintSide1Pattern();
                Serial.println("Front side painting completed");
                return;
              }
              else if (side == "side3") {
                message = "Painting back side...";
                dashboardClient.println("HTTP/1.1 200 OK");
                dashboardClient.println("Content-Type: text/html");
                dashboardClient.println("Connection: close");
                dashboardClient.println();
                
                String response = String(response_html);
                response.replace("%MESSAGE%", message);
                dashboardClient.println(response);
                
                dashboardClient.flush();
                dashboardClient.stop();
                
                // Now paint
                Serial.println(message);
                paintSide3Pattern();
                Serial.println("Back side painting completed");
                return;
              }
              else if (side == "all") {
                message = "Painting all sides...";
                dashboardClient.println("HTTP/1.1 200 OK");
                dashboardClient.println("Content-Type: text/html");
                dashboardClient.println("Connection: close");
                dashboardClient.println();
                
                String response = String(response_html);
                response.replace("%MESSAGE%", message);
                dashboardClient.println(response);
              }
              break;
            }
            else {
              // Not found
              dashboardClient.println("HTTP/1.1 404 Not Found");
              dashboardClient.println("Content-Type: text/html");
              dashboardClient.println("Connection: close");
              dashboardClient.println();
              dashboardClient.println("<html><body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p><a href='/'>Back to Dashboard</a></body></html>");
              break;
            }
          } else {
            // If we got a newline, check if this is the request line
            if (currentLine.startsWith("GET ")) {
              request = currentLine;
            }
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    
    // Close the connection
    dashboardClient.stop();
  }
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
WiFiServer server(80);
String header;

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