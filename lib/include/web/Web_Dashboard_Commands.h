#ifndef WEB_DASHBOARD_COMMANDS_H
#define WEB_DASHBOARD_COMMANDS_H

#include <Arduino.h>
#include <WebSocketsServer.h> // Needed for WebSocketsServer type
#include <WiFiServer.h>      // Needed for WiFiServer type

// Declare functions defined in Web_Dashboard_Commands.cpp that are used elsewhere

// Main function to handle web server and WebSocket loop
void runDashboardServer();

// Function to stop the server
void stopDashboardServer();

// Function to handle WebSocket events (referenced by Setup.cpp)
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

// Function to send status updates (used by other modules like paintGun)
// Note: This requires the webSocket object. It might be better to pass it as an argument
// or have a dedicated messaging module.
void sendWebStatus(WebSocketsServer* webSocket, const char* message);

// Process WebSocket events without executing commands (for checking during operations)
void processWebSocketEvents();

// Check if a home command was received during painting operations
bool checkForHomeCommand();

#endif // WEB_DASHBOARD_COMMANDS_H 