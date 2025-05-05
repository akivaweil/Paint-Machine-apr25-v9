#ifndef HTML_CONTENT_H
#define HTML_CONTENT_H
#include <Arduino.h>

const char HTML_PROGMEM[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <title>ESP32 Stepper Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        /* Ultra-Clean Professional Mode Styles */
        
        body {
            background: #151619;
            color: #EEE;
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 15px;
            /* Center main content blocks */
            display: flex;
            flex-direction: column;
            align-items: center;
            font-size: 1.5em;
        }
        
        .container {
            max-width: 1000px;
            margin: 0 auto;
        }
        
        h1 {
            color: #4CAF50;
            font-size: 1.5em;
        }
        
        button,
        input[type=button] {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 4px 4px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 24px;
            margin: 6px;
            cursor: pointer;
            border-radius: 6px;
        }
        
        .main-card {
            background: #1e1f24;
            border: 1px solid #333;
            border-radius: 14px;
            padding: 20px 25px;
            max-width: 690px;
            margin: 0 auto 25px auto;
            box-shadow: 0 8px 24px rgba(0, 0, 0, .25);
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        
        .main-divider {
            width: 100%;
            height: 1px;
            background: linear-gradient(90deg, #2d2e35 0%, #3c3d44 100%);
            margin: 10px 0;
            border: none;
        }
        
        .main-btn {
            all: unset;
            position: relative;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            gap: 7px;
            padding: 12px 18px;
            border-radius: 7px;
            background: linear-gradient(90deg, #48a04c 60%, #3c863f 100%);
            color: #fff;
            font-size: 1.05em;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: .04em;
            cursor: pointer;
            box-shadow: 0 2px 4px rgba(0, 0, 0, .25), inset 0 1px 0 rgba(255, 255, 255, .05);
            transition: background .2s ease-in-out, transform .15s ease-in-out, box-shadow .2s ease-in-out;
            min-width: 130px;
            outline: none;
        }
        
        .main-btn .btn-icon {
            font-size: 1.25em;
            display: inline-block
        }
        
        .main-btn:hover,
        .main-btn:focus {
            background: linear-gradient(90deg, #5cb860 60%, #4caf50 100%);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0, 0, 0, .3), inset 0 1px 0 rgba(255, 255, 255, .07);
            outline: none !important;
        }
        
        .main-btn:active {
            transform: scale(.98);
            transition-duration: 0.05s;
            outline: none !important;
        }
        
        .main-btn:disabled {
            background: #52525b;
            cursor: not-allowed;
            box-shadow: none;
            transform: none;
            outline: none !important;
        }
        
        .main-btn.highlight {
            background: linear-gradient(90deg, #fbc02d 60%, #f9a825 100%);
            color: #222;
        }
        
        .main-btn.highlight:hover,
        .main-btn.highlight:focus {
            background: linear-gradient(90deg, #fdd835 60%, #ffb300 100%);
            color: #111;
            outline: none !important;
        }
        
        .main-btn.blue {
            background: linear-gradient(90deg, #26c6da 60%, #039be5 100%);
        }
        
        .main-btn.blue:hover,
        .main-btn.blue:focus {
            background: linear-gradient(90deg, #4dd0e1 60%, #29b6f6 100%);
            outline: none !important;
        }
        
        .main-btn.mode {
            background: linear-gradient(90deg, #42a5f5 60%, #1e88e5 100%);
        }
        
        .main-btn.mode:hover,
        .main-btn.mode:focus {
            background: linear-gradient(90deg, #64b5f6 60%, #42a5f5 100%);
            outline: none !important;
        }
        
        @media (max-width: 600px) {
            .main-card {
                padding: 18px 6px;
                min-width: 0;
                max-width: 100vw;
            }
            .main-controls-grid {
                gap: 6px;
            }
            .main-btn {
                min-width: 90px;
                font-size: 0.98em;
                padding: 10px 8px;
            }
        }
        
        /* Toggle Switch for Pressure Pot */
        .toggle-switch {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 12px;
            cursor: pointer;
            user-select: none;
            margin: 0 auto;
            padding: 5px 0 8px 0;
        }
        
        .toggle-switch input[type="checkbox"] {
            display: none;
        }
        
        .toggle-switch .slider {
            position: relative;
            width: 60px;
            height: 34px;
            background: #3a3b41;
            border-radius: 17px;
            transition: all 0.25s ease-in-out;
            box-shadow: inset 0 2px 8px rgba(0, 0, 0, 0.15);
        }
        
        .toggle-switch .slider:before {
            content: "";
            position: absolute;
            left: 4px;
            top: 4px;
            width: 26px;
            height: 26px;
            background: #fff;
            border-radius: 50%;
            transition: transform 0.25s ease-in-out;
            box-shadow: 0 2px 6px rgba(0, 0, 0, 0.2);
        }
        
        .toggle-switch input:checked+.slider {
            background: linear-gradient(90deg, #a142f4 60%, #9138e0 100%);
        }
        
        .toggle-switch input:checked+.slider:before {
            transform: translateX(26px);
        }
        
        .toggle-label {
            min-width: 40px;
            font-weight: 600;
            color: #9ca3af;
            letter-spacing: 0.04em;
            font-size: 1.1em;
            transition: color 0.2s;
            text-align: center;
        }
        
        .toggle-switch input:checked~.toggle-label {
            color: #a142f4;
        }
        
        /* Top Controls Container */
        .top-controls-container {
            display: flex;
            justify-content: center; /* Center items horizontally */
            align-items: flex-start; /* Align items to the top */
            flex-wrap: wrap; /* Allow items to wrap */
            gap: 25px; /* Increased gap for better spacing when wrapped */
            /* Removed margin: 0 auto; as justify-content handles centering */
            margin-bottom: 25px; /* Adjusted bottom margin */
            padding-top: 10px;
            width: 100%;
            max-width: 1100px;
        }
        
        /* Pressure Controls - Now a flex item */
        #pressureControlGroup {
             /* Inherits from .main-card */
            flex: 0 0 auto; /* Don't grow, don't shrink, use auto basis (effectively max-width) */
            padding: 14px 16px; 
            width: 240px; /* Explicit width instead of max-width */
            margin: 0; /* Remove margin-left, gap handles spacing */
            align-self: stretch; /* Make it same height as main card if possible */
        }
        
        #pressureControlGroup h2 {
            color: #a142f4;
            font-size: 1.2em;
            margin: 0 0 20px 0;
            letter-spacing: 0.02em;
            text-align: center;
        }
        
        .toggle-container {
            width: 100%;
            padding: 8px 12px;
            margin-bottom: 12px;
            border-radius: 10px;
            background: rgba(255, 255, 255, 0.03);
            border: 1px solid rgba(161, 66, 244, 0.2);
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        
        .toggle-container:last-child {
            margin-bottom: 0;
        }
        
        .toggle-container h3 {
            color: #9ca3af;
            font-size: 1.05em;
            margin: 0 0 8px 0;
            letter-spacing: 0.02em;
            text-align: center;
        }
        
        /* Machine Status Display - Adjusted for positioning above flex */
        #machineStatusDisplay {
            background: rgba(255, 255, 255, 0.06);
            border: 1px solid #444;
            border-radius: 10px;
            padding: 12px 20px;
            margin: 0 auto 20px auto; /* Centered, with bottom margin */
            font-size: 1.1em;
            font-weight: 500;
            color: #b0b8c4;
            text-align: center;
            min-height: 25px; 
            width: 95%; /* Use percentage width */
            max-width: 800px; /* Increased max-width */
            box-sizing: border-box; 
            display: block; /* Ensure it's a block element */
        }
        
        /* Integrated main card - Now a flex item */
        .integrated-main-card {
             /* Inherits from .main-card */
            flex: 1 1 520px; /* Grow, shrink, preferred basis 520px */
            border: 1px solid rgba(161, 66, 244, 0.2);
            padding: 25px; 
            box-shadow: 0 8px 24px rgba(0, 0, 0, .25);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 15px;
            min-width: 300px; /* Prevent excessive shrinking */
            max-width: 600px; /* Allow growing slightly larger */
             /* width: 100%; Removed, flex basis handles this */
             /* max-width: 520px; Replaced by flex-basis */
             margin: 0; /* Remove margins, gap handles spacing */
        }

        /* Grid layout for the directional buttons */
        .direction-buttons-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            grid-template-rows: repeat(3, auto);
            gap: 10px;
            width: 100%;
            place-items: center;
            align-content: center;
            justify-content: center;
        }

        /* Grid layout for the utility buttons */
        .utility-buttons-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            width: 100%;
            place-items: center;
            align-content: center;
            justify-content: center;
        }

        /* Make sure the utility buttons have consistent height */
        .utility-buttons-grid .main-btn {
            height: 60px; /* Set a fixed height */
            width: 100%; /* Make buttons fill grid cell */
            box-sizing: border-box; /* Include padding in width */
            padding: 14px 20px;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        /* Pattern Settings Styles */
        .pattern-settings-container {
            max-width: 900px;
            margin: 20px auto;
            padding-top: 15px;
            padding-bottom: 15px;
        }
        
        .pattern-settings-header {
            color: #4CAF50;
            font-size: 1.5em;
            margin-bottom: 15px;
            cursor: pointer;
            user-select: none; /* Prevent text selection on click */
            display: flex; /* Use flexbox for alignment */
            justify-content: space-between; /* Space between title and indicator */
            align-items: center; /* Vertically align items */
            padding: 10px 15px; /* Add some padding */
            background: rgba(255, 255, 255, 0.04); /* Slight background */
            border-radius: 8px; /* Rounded corners */
            border: 1px solid rgba(76, 175, 80, 0.2); /* Subtle border */
            transition: background-color 0.2s;
        }

        .pattern-settings-header:hover {
            background: rgba(255, 255, 255, 0.08); /* Lighter background on hover */
        }

        .pattern-settings-header .toggle-indicator {
            font-size: 0.8em;
            transition: transform 0.3s ease-in-out;
        }

        .pattern-settings-content-wrapper {
            background: #1e1f24;
            border: 1px solid #333;
            border-radius: 10px;
            padding: 18px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, .2);
            overflow: hidden;
            transition: max-height 0.35s ease-in-out, padding 0.35s ease-in-out, border-width 0.35s ease-in-out;
            max-height: 2000px;
        }

        .pattern-settings-content-wrapper.collapsed {
            max-height: 0;
            padding-top: 0;
            padding-bottom: 0;
            border-top-width: 0;
            border-bottom-width: 0;
            box-shadow: none;
        }

        .pattern-settings-container h2 {
            color: #4CAF50;
            font-size: 1.5em;
            margin-bottom: 20px;
        }
        
        /* Tab Navigation */
        .pattern-tabs {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 10px;
            margin-bottom: 15px;
        }
        
        .pattern-tab {
            background-color: #333;
            color: #EEE;
            border: none;
            padding: 10px 18px;
            font-size: 1em;
            border-radius: 8px;
            cursor: pointer;
            transition: background-color 0.3s, transform 0.2s;
        }
        
        .pattern-tab:hover {
            background-color: #444;
        }
        
        .pattern-tab.active {
            background-color: #4CAF50;
            transform: translateY(-3px);
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
        }
        
        /* Tab Content */
        .pattern-tab-content {
            display: none;
            width: 100%;
        }
        
        .pattern-tab-content.active {
            display: block;
        }
        
        /* Pattern Settings Grid */
        .pattern-settings-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 15px;
            margin-bottom: 15px;
        }
        
        .pattern-setting-group {
            background: rgba(255, 255, 255, 0.04);
            border-radius: 6px;
            padding: 12px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        
        .pattern-setting-group h3 {
            color: #9ca3af;
            font-size: 0.9em;
            margin-top: 0;
            margin-bottom: 8px;
        }
        
        .setting-input {
            background: #2c2d32;
            color: #EEE;
            border: 1px solid #4a4a50;
            border-radius: 5px;
            padding: 7px 10px;
            font-size: 0.95em;
            width: 90px;
            text-align: center;
        }
        
        /* Hide number input spinner arrows */
        input[type="number"]::-webkit-inner-spin-button,
        input[type="number"]::-webkit-outer-spin-button {
            -webkit-appearance: none;
            margin: 0;
        }
        
        input[type="number"] {
            -moz-appearance: textfield;
        }
        
        .labeled-inputs {
            display: flex;
            align-items: center; /* Vertically align label and input */
            justify-content: center; /* Keep centered layout */
            gap: 5px; /* Add small gap between label and input */
            flex-wrap: wrap; /* Allow wrapping if needed */
        }

        .labeled-inputs label {
            margin-right: 3px; /* Space between label and input box */
            font-size: 0.9em; /* Slightly smaller label */
            color: #b0b8c4; /* Match other text color */
        }

        .labeled-inputs .setting-input {
            width: 70px; /* Adjust width slightly if needed */
        }

        .setting-inputs {
            display: flex;
            gap: 10px;
            width: 100%;
            justify-content: center;
        }
        
        .settings-controls {
            display: flex;
            justify-content: center;
            gap: 30px;
            margin-top: 15px;
        }
        
        .settings-controls .main-btn {
            min-width: 160px;
            padding: 12px 22px;
            font-size: 1em;
        }
        
        /* Responsive Design */
        @media (max-width: 768px) {
            .pattern-settings-container {
                max-width: 95%;
                padding: 15px;
            }
            
            .pattern-settings-grid {
                grid-template-columns: repeat(2, 1fr);
                gap: 15px;
            }
            
            .pattern-setting-group {
                padding: 15px 10px;
            }
        }
        
        @media (max-width: 480px) {
            .pattern-settings-grid {
                grid-template-columns: 1fr;
            }
            
            .pattern-tabs {
                flex-direction: column;
                align-items: stretch;
            }
            
            .pattern-setting-group {
                padding: 12px 8px;
            }
            
            .setting-input {
                width: 90px;
            }
        }
    </style>
    <script>
        // Global variables
        var websocket;
        var isWebSocketConnected = false;
        var reconnectInterval;
        var connectionAttempts = 0;
        var connectionStatus = document.createElement('div');
        
        // Initialize WebSocket connection
        function initWebSocket() {
            // Create status indicator if it doesn't exist
            if (!document.getElementById('connectionStatus')) {
                connectionStatus.id = 'connectionStatus';
                connectionStatus.style.position = 'fixed';
                connectionStatus.style.top = '10px';
                connectionStatus.style.right = '10px';
                connectionStatus.style.padding = '5px 10px';
                connectionStatus.style.borderRadius = '5px';
                connectionStatus.style.fontSize = '12px';
                connectionStatus.style.fontWeight = 'bold';
                connectionStatus.style.zIndex = '1000';
                connectionStatus.style.backgroundColor = '#ff5555';
                connectionStatus.style.color = 'white';
                connectionStatus.textContent = 'Connecting...';
                document.body.appendChild(connectionStatus);
            }
            
            // Update status
            updateConnectionStatus('connecting');
            
            // Use dynamic hostname instead of hardcoded IP
            var wsProtocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            var wsHost = window.location.hostname + ':81';
            var gateway = wsProtocol + '//' + wsHost + '/';
            
            console.log('Trying to open a WebSocket connection to: ' + gateway);
            
            try {
                websocket = new WebSocket(gateway);
                
                websocket.onopen = function(event) {
                    console.log('Connection opened');
                    isWebSocketConnected = true;
                    connectionAttempts = 0;
                    clearInterval(reconnectInterval);
                    updateConnectionStatus('connected');
                    
                    // Request current status after connection is established
                    setTimeout(function() {
                        sendCommand('GET_STATUS');
                        
                        // If we're initializing pattern settings, load them now
                        if (window.needToLoadPatternSettings) {
                            window.needToLoadPatternSettings = false;
                            sendCommand('GET_PAINT_SETTINGS');
                        }
                    }, 500);
                };
                
                websocket.onclose = function(event) {
                    console.log('Connection closed');
                    isWebSocketConnected = false;
                    updateConnectionStatus('disconnected');
                    scheduleReconnect();
                };
                
                websocket.onmessage = function(event) {
                    console.log('Received message: ' + event.data);
                    handleWebSocketMessage(event.data);
                };
                
                websocket.onerror = function(event) {
                    console.error('WebSocket error: ', event);
                    isWebSocketConnected = false;
                    updateConnectionStatus('error');
                    scheduleReconnect();
                };
            } catch (e) {
                console.error('Exception creating WebSocket: ' + e.message);
                updateConnectionStatus('error', e.message);
                scheduleReconnect();
            }
        }
        
        // Update connection status indicator
        function updateConnectionStatus(status, message) {
            const connectionStatus = document.getElementById('connectionStatus');
            if (!connectionStatus) {
                console.error("Connection status element not found!");
                return;
            }
            switch(status) {
                case 'connected':
                    connectionStatus.style.backgroundColor = '#4CAF50';
                    connectionStatus.textContent = 'Connected';
                    connectionStatus.style.opacity = '0.8';
                    // Fade out after 3 seconds
                    setTimeout(function() {
                        connectionStatus.style.opacity = '0.3';
                    }, 3000);
                    break;
                case 'disconnected':
                    connectionStatus.style.backgroundColor = '#ff5555';
                    connectionStatus.textContent = 'Disconnected - Reconnecting...';
                    connectionStatus.style.opacity = '1';
                    break;
                case 'connecting':
                    connectionStatus.style.backgroundColor = '#FFA500';
                    connectionStatus.textContent = 'Connecting...';
                    connectionStatus.style.opacity = '1';
                    break;
                case 'error':
                    connectionStatus.style.backgroundColor = '#ff5555';
                    connectionStatus.textContent = 'Connection Error' + (message ? ': ' + message : '');
                    connectionStatus.style.opacity = '1';
                    break;
            }
        }
        
        // Schedule reconnection with tiered intervals and timeout
        function scheduleReconnect() {
            if (reconnectInterval) {
                clearInterval(reconnectInterval);
            }

            connectionAttempts++;
            var delay;

            // Tier 1: 0.25s interval for the first 60 seconds (240 attempts)
            if (connectionAttempts <= 240) {
                delay = 250;
            } 
            // Tier 2: 0.5s interval for the next 120 seconds (up to 3 min total, 480 attempts)
            else if (connectionAttempts <= 480) {
                delay = 500;
            } 
            // Stop trying after 3 minutes
            else {
                console.log("Stopped attempting to reconnect after 3 minutes.");
                updateConnectionStatus('error', 'Stopped reconnecting after 3 min timeout.'); // Update status
                return; // Do not schedule another attempt
            }

            console.log(`Scheduling reconnect in ${delay}ms (attempt ${connectionAttempts})`);

            reconnectInterval = setTimeout(function() {
                if (!isWebSocketConnected) {
                    initWebSocket();
                }
            }, delay);
        }
        
        // Handle incoming WebSocket messages
        function handleWebSocketMessage(message) {
            // Process messages from the ESP32
            try {
                console.log('Processing message: ' + message);
                
                // --- State Handling --- 
                if (message.startsWith('STATE:')) {
                    const stateName = message.substring(6); // Get text after "STATE:"
                    const statusDisplay = document.getElementById('machineStatusDisplay');
                    if (statusDisplay) {
                        statusDisplay.textContent = stateName; // Update status display
                        console.log('Updated machine status display to: ' + stateName);
                    }
                    updateButtonStates(stateName); // Enable/disable buttons based on state
                }
                // Check for machine status updates (REMOVED - Handled by STATE: now)
                // else if (message.startsWith('MACHINE_STATUS:')) { ... }
                
                // Check for pressure pot status updates
                else if (message.startsWith('PRESSURE_POT_STATUS:')) {
                    const status = message.split(':')[1];
                    console.log('Updating pressure pot status to: ' + status);
                    
                    // Update UI toggle without triggering a new command
                    const toggle = document.getElementById('pressurePotToggle');
                    const label = document.getElementById('pressurePotToggleLabel');
                    
                    if (status === 'ON') {
                        toggle.checked = true;
                        label.textContent = 'ON';
                    } else {
                        toggle.checked = false;
                        label.textContent = 'OFF';
                    }
                }
                
                // Check for paint gun status updates
                else if (message.startsWith('PAINT_GUN_STATUS:')) {
                    const status = message.split(':')[1];
                    console.log('Updating paint gun status to: ' + status);
                    
                    // Update UI toggle without triggering a new command
                    const toggle = document.getElementById('paintGunToggle');
                    const label = document.getElementById('paintGunToggleLabel');
                    
                    if (status === 'ON') {
                        toggle.checked = true;
                        label.textContent = 'ON';
                    } else {
                        toggle.checked = false;
                        label.textContent = 'OFF';
                    }
                }
                
                // Handle settings messages
                else if (message.startsWith('SETTING:')) {
                    const parts = message.split(':');
                    if (parts.length >= 3) {
                        const setting = parts[1];
                        const value = parts[2];
                        updateSettingField(setting, value);
                    }
                }
                
                // Handle status messages
                else if (message.startsWith('STATUS:')) {
                    console.log('Status message: ' + message.substring(7));
                    // Could add notification display here
                }
            } catch (e) {
                console.error('Error handling message: ' + e.message);
            }
        }
        
        // --- Function to Enable/Disable Buttons based on State --- 
        function updateButtonStates(stateName) {
            console.log("Updating button states for state: " + stateName);
            
            // Determine if buttons should be enabled (Machine is idle OR in PnP mode)
            const canPerformActions = (stateName === 'IDLE' || stateName === 'PNP'); 
            
            // Get button elements
            const paintSide1Btn = document.getElementById('paintSide1Btn');
            const paintSide2Btn = document.getElementById('paintSide2Btn');
            const paintSide3Btn = document.getElementById('paintSide3Btn');
            const paintSide4Btn = document.getElementById('paintSide4Btn');
            const paintAllSidesBtn = document.getElementById('paintAllSidesBtn');
            const homeBtn = document.getElementById('homeBtn');
            const cleanGunBtn = document.getElementById('cleanGunBtn');
            const pnpButton = document.getElementById('pnpButton');
            const pressurePotToggle = document.getElementById('pressurePotToggle');
            const paintGunToggle = document.getElementById('paintGunToggle');
            
            // Action buttons (Painting, Homing, Cleaning, PnP Start) 
            // Enabled only when IDLE or PNP
            if (paintSide1Btn) paintSide1Btn.disabled = !canPerformActions;
            if (paintSide2Btn) paintSide2Btn.disabled = !canPerformActions;
            if (paintSide3Btn) paintSide3Btn.disabled = !canPerformActions;
            if (paintSide4Btn) paintSide4Btn.disabled = !canPerformActions;
            if (paintAllSidesBtn) paintAllSidesBtn.disabled = !canPerformActions;
            if (homeBtn) homeBtn.disabled = !canPerformActions;
            if (cleanGunBtn) cleanGunBtn.disabled = !canPerformActions;
            if (pnpButton) pnpButton.disabled = !canPerformActions;
            
            // Toggles (Pressure Pot, Paint Gun) - Generally always enabled, 
            // but you might want to disable them during certain states too?
            // For now, keeping them enabled.
            // if (pressurePotToggle) pressurePotToggle.disabled = !canPerformActions; // Example if needed
            // if (paintGunToggle) paintGunToggle.disabled = !canPerformActions; // Example if needed
        }

        // Send commands to the ESP32
        function sendCommand(command) {
            if (!websocket || websocket.readyState !== WebSocket.OPEN) {
                console.error("WebSocket is not connected!");
                updateConnectionStatus('disconnected');
                
                // Try to reconnect immediately
                if (!reconnectInterval) {
                    initWebSocket();
                }
                
                // Store the command to send after reconnection
                if (command === 'GET_PAINT_SETTINGS') {
                    window.needToLoadPatternSettings = true;
                } else if (command !== 'GET_STATUS') {  // Don't queue status requests
                    setTimeout(function() {
                        if (isWebSocketConnected) {
                            console.log(`Resending queued command: ${command}`);
                            websocket.send(command);
                        }
                    }, 1500);
                }
                return false;
            }
            
            try {
                websocket.send(command);
                console.log(`Command sent: ${command}`);
                return true;
            } catch (e) {
                console.error(`Error sending command: ${e.message}`);
                updateConnectionStatus('error', e.message);
                return false;
            }
        }
        
        // Toggle Pressure Pot
        function togglePressurePot(enabled) {
            // Send the command to the device
            const command = enabled ? 'PRESSURE_POT_ON' : 'PRESSURE_POT_OFF';
            sendCommand(command);
            
            // Update the UI explicitly
            const toggle = document.getElementById('pressurePotToggle');
            const label = document.getElementById('pressurePotToggleLabel');
            
            // Set the toggle state
            toggle.checked = enabled;
            
            // Update the label
            label.textContent = enabled ? 'ON' : 'OFF';
            
            // Debug toggle state
            console.log(`Toggle Pressure Pot: ${enabled ? 'ON' : 'OFF'}, checked=${toggle.checked}`);
        }
        
        // Toggle Paint Gun
        function togglePaintGun(enabled) {
            // Send the command to the device
            const command = enabled ? 'PAINT_GUN_ON' : 'PAINT_GUN_OFF';
            sendCommand(command);
            
            // Update the UI explicitly
            const toggle = document.getElementById('paintGunToggle');
            const label = document.getElementById('paintGunToggleLabel');
            
            // Set the toggle state
            toggle.checked = enabled;
            
            // Update the label
            label.textContent = enabled ? 'ON' : 'OFF';
            
            // Debug toggle state
            console.log(`Toggle Paint Gun: ${enabled ? 'ON' : 'OFF'}, checked=${toggle.checked}`);
        }

        // Handle Pattern Tab Navigation
        function openPatternTab(tabId) {
            // Hide all tab contents
            var tabContents = document.getElementsByClassName('pattern-tab-content');
            for (var i = 0; i < tabContents.length; i++) {
                tabContents[i].classList.remove('active');
            }
            
            // Deactivate all tab buttons
            var tabButtons = document.getElementsByClassName('pattern-tab');
            for (var i = 0; i < tabButtons.length; i++) {
                tabButtons[i].classList.remove('active');
            }
            
            // Show the selected tab content
            document.getElementById(tabId).classList.add('active');
            
            // Activate the clicked tab button
            document.getElementById(tabId + 'Btn').classList.add('active');
        }
        
        // Update a pattern setting and send to the ESP32
        function updatePatternSetting(setting, value) {
            console.log(`Updating ${setting} to ${value}`);
            sendCommand(`SET_${setting}:${value}`);

            // Add preview commands for Z height and Servo angle
            if (setting.endsWith('ZHEIGHT')) {
                sendCommand(`MOVE_Z_PREVIEW:${value}`);
            } else if (setting.startsWith('SERVO_ANGLE')) {
                sendCommand(`MOVE_SERVO_PREVIEW:${value}`);
            }
        }
        
        // Load all pattern settings from the ESP32
        function loadPatternSettings() {
            console.log("Loading pattern settings");
            sendCommand('GET_PAINT_SETTINGS');
        }
        
        // Save all pattern settings to NVS
        function savePatternSettings() {
            console.log("Saving pattern settings");
            sendCommand('SAVE_PAINT_SETTINGS');
        }
        
        // Handle incoming settings from the ESP32
        function updateSettingField(setting, value) {
            const field = document.getElementById(setting);
            if (field) {
                field.value = value;
                console.log(`Updated ${setting} field to ${value}`);
            }
        }
        
        // Initialize pattern settings section when page loads
        function initPatternSettings() {
            // Activate the first tab by default
            openPatternTab('side3Tab');
            
            // Make settings section collapsible
            const header = document.getElementById('patternSettingsHeader');
            const content = document.getElementById('patternSettingsContent');
            if(header && content) {
                 // Start collapsed by default - REMOVED these lines
                 // header.classList.add('collapsed');
                 // content.classList.add('collapsed');

                 header.onclick = function() {
                    header.classList.toggle('collapsed');
                    content.classList.toggle('collapsed');
                 };
            } else {
                console.error("Could not find pattern settings header or content for collapsible feature.");
            }

            // Flag that we need to load settings once connected
            window.needToLoadPatternSettings = true;
            
            // If already connected, load immediately
            if (isWebSocketConnected && websocket && websocket.readyState === WebSocket.OPEN) {
                console.log("Loading pattern settings");
                window.needToLoadPatternSettings = false;
                sendCommand('GET_PAINT_SETTINGS');
            }
        }

        // Initialize the app when the page loads
        window.addEventListener('load', function() {
            initWebSocket();
            initPatternSettings();
        });
    </script>
</head>

<body>
    <!-- Machine Status Display (Moved outside top-controls-container) -->
    <div id="machineStatusDisplay">Machine status will appear here...</div>

    <!-- Main Controls Container -->
    <div class="top-controls-container">
        <!-- Integrated Main Control Card with all primary buttons -->
        <div class="integrated-main-card main-card">
            <!-- Paint Direction Buttons -->
            <div class="direction-buttons-grid">
                <!-- SIDE 1 button at top center -->
                <button id="paintSide1Btn" class="main-btn" title="Paint Side 1" aria-label="Paint Side 1" onclick="sendCommand('PAINT_SIDE_1')" style="grid-column: 2; grid-row: 1;">
                    <span class="btn-icon" aria-hidden="true">
                        <!-- SVG: Arrow Down -->
                        <svg width="30" height="30" viewBox="0 0 30 30" fill="none"><path d="M10 4v12M10 16l6-6M10 16l-6-6" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></svg>
                    </span>
                    <span class="btn-label">SIDE 1</span>
                </button>

                <!-- SIDE 4 button on left side -->
                <button id="paintSide4Btn" class="main-btn" title="Paint Side 4" aria-label="Paint Side 4" onclick="sendCommand('PAINT_SIDE_4')" style="grid-column: 1; grid-row: 2;">
                    <span class="btn-icon" aria-hidden="true">
                        <!-- SVG: Arrow Right -->
                        <svg width="30" height="30" viewBox="0 0 30 30" fill="none"><path d="M4 10h12M16 10l-6-6M16 10l-6 6" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></svg>
                    </span>
                    <span class="btn-label">SIDE 4</span>
                </button>

                <!-- ALL SIDES button in center -->
                <button id="paintAllSidesBtn" class="main-btn highlight" title="Paint All Sides" aria-label="Paint All Sides" onclick="sendCommand('PAINT_ALL_SIDES')" style="grid-column: 2; grid-row: 2;">
                    <span class="btn-icon" aria-hidden="true">
                        <!-- central rectangle with four arrows pointing inward -->
                        <svg width="39" height="39" viewBox="0 0 39 39" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <!-- vertical rectangle -->
                            <rect x="6" y="0" width="27" height="39" rx="3" fill="#111" stroke="#222" stroke-width="1.5"/>
                            <!-- top arrow -->
                            <path d="M19.5 0v6M19.5 6l-3-3M19.5 6l3-3"/>
                            <!-- bottom arrow -->
                            <path d="M19.5 39v-6M19.5 33l-3 3M19.5 33l3 3"/>
                            <!-- left arrow -->
                            <path d="M0 19.5h6M6 19.5l-3-3M6 19.5l-3 3"/>
                            <!-- right arrow -->
                            <path d="M39 19.5h-6M33 19.5l3-3M33 19.5l3 3"/>
                        </svg>
                    </span>
                    <span class="btn-label">ALL SIDES</span>
                </button>

                <!-- SIDE 2 button on right side -->
                <button id="paintSide2Btn" class="main-btn" title="Paint Side 2" aria-label="Paint Side 2" onclick="sendCommand('PAINT_SIDE_2')" style="grid-column: 3; grid-row: 2;">
                    <span class="btn-icon" aria-hidden="true">
                        <!-- SVG: Arrow Left -->
                        <svg width="30" height="30" viewBox="0 0 30 30" fill="none"><path d="M16 10H4M4 10l6-6M4 10l6 6" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></svg>
                    </span>
                    <span class="btn-label">SIDE 2</span>
                </button>

                <!-- SIDE 3 button at bottom center -->
                <button id="paintSide3Btn" class="main-btn" title="Paint Side 3" aria-label="Paint Side 3" onclick="sendCommand('PAINT_SIDE_3')" style="grid-column: 2; grid-row: 3;">
                    <span class="btn-icon" aria-hidden="true">
                        <!-- SVG: Arrow Up -->
                        <svg width="30" height="30" viewBox="0 0 30 30" fill="none"><path d="M10 16V4M10 4l6 6M10 4l-6 6" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></svg>
                    </span>
                    <span class="btn-label">SIDE 3</span>
                </button>
            </div>

            <div class="main-divider"></div>

            <!-- Utility Buttons -->
            <div class="utility-buttons-grid">
                <button id="homeBtn" class="main-btn blue" title="Home All Axes" aria-label="Home" onclick="sendCommand('HOME')">
                    <span class="btn-icon" aria-hidden="true">
                        <!-- SVG: Home -->
                        <svg width="30" height="30" viewBox="0 0 30 30" fill="none"><path d="M3 10l7-7 7 7M5 8v7a1 1 0 001 1h8a1 1 0 001-1V8" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></svg>
                    </span>
                    <span class="btn-label">HOME</span>
                </button>
                <button id="cleanGunBtn" class="main-btn blue" title="Clean Paint Gun" aria-label="Clean Gun" onclick="sendCommand('CLEAN_GUN')">
                    <span class="btn-icon" aria-hidden="true">
                        <!-- SVG: Sparkle/Clean -->
                        <svg width="30" height="30" viewBox="0 0 30 30" fill="none"><circle cx="10" cy="10" r="7" stroke="currentColor" stroke-width="2"/><path d="M10 5v2M10 13v2M5 10h2M13 10h2M7.05 7.05l1.41 1.41M11.54 11.54l1.41 1.41M7.05 12.95l1.41-1.41M11.54 8.46l1.41-1.41" stroke="currentColor" stroke-width="2" stroke-linecap="round"/></svg>
                    </span>
                    <span class="btn-label">CLEAN GUN</span>
                </button>
                <button id="pnpButton" class="main-btn mode" title="Switch to Pick and Place Mode" aria-label="Pick and Place Mode" onclick="sendCommand('ENTER_PICKPLACE')">
                    <span class="btn-icon" aria-hidden="true">
                        <!-- SVG: Robot -->
                        <svg width="30" height="30" viewBox="0 0 30 30" fill="none"><rect x="4" y="8" width="12" height="8" rx="2" stroke="currentColor" stroke-width="2"/><circle cx="7" cy="12" r="1" fill="currentColor"/><circle cx="13" cy="12" r="1" fill="currentColor"/><rect x="8" y="4" width="4" height="4" rx="2" stroke="currentColor" stroke-width="2"/></svg>
                    </span>
                    <span class="btn-label">PICK & PLACE</span>
                </button>
            </div>
        </div>

        <!-- Pressure Control Group -->
        <div class="input-group section-paint main-card" id="pressureControlGroup">
            <h2>Pressure/Paint</h2>
            <div class="toggle-container">
                <h3>Pressure Pot</h3>
                <label class="toggle-switch">
                    <input type="checkbox" id="pressurePotToggle" onchange="togglePressurePot(this.checked)">
                    <span class="slider"></span>
                    <span class="toggle-label" id="pressurePotToggleLabel">OFF</span>
                </label>
            </div>

            <div class="toggle-container">
                <h3>Paint Gun</h3>
                <label class="toggle-switch">
                    <input type="checkbox" id="paintGunToggle" onchange="togglePaintGun(this.checked)">
                    <span class="slider"></span>
                    <span class="toggle-label" id="paintGunToggleLabel">OFF</span>
                </label>
            </div>
        </div>
    </div>
    
    <!-- Pattern Settings Section with Tabs -->
    <div class="pattern-settings-container">
        <!-- Clickable Header for Toggling -->
         <h2 id="patternSettingsHeader" class="pattern-settings-header">
            Pattern Settings
         </h2>

        <!-- Content Wrapper for Collapsing -->
        <div class="pattern-settings-content-wrapper" id="patternSettingsContent">
            <!-- Tab Navigation -->
            <div class="pattern-tabs">
                <button class="pattern-tab" id="side3TabBtn" onclick="openPatternTab('side3Tab')">Side 3 Settings</button>
                <button class="pattern-tab" id="side1TabBtn" onclick="openPatternTab('side1Tab')">Side 1 Settings</button>
                <button class="pattern-tab" id="side4TabBtn" onclick="openPatternTab('side4Tab')">Side 4 Settings</button>
                <button class="pattern-tab" id="side2TabBtn" onclick="openPatternTab('side2Tab')">Side 2 Settings</button>
            </div>
            
            <div class="main-divider"></div>
            
            <!-- Side 3 Settings Tab Content -->
            <div class="pattern-tab-content" id="side3Tab">
                <div class="pattern-settings-grid">
                    <div class="pattern-setting-group">
                        <h3>Side 3 Starting Position</h3>
                        <div class="setting-inputs labeled-inputs">
                            <label for="side3StartX">X:</label>
                            <input type="number" id="side3StartX" class="setting-input" min="0" max="37" placeholder="0.0" onchange="updatePatternSetting('SIDE3STARTX', this.value)">
                            <label for="side3StartY">Y:</label>
                            <input type="number" id="side3StartY" class="setting-input" min="0" max="37" placeholder="0.0" onchange="updatePatternSetting('SIDE3STARTY', this.value)">
                        </div>
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Shift Distance</h3>
                        <input type="number" id="side3ShiftX" class="setting-input" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE3SHIFTX', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Sweep Distance</h3>
                        <input type="number" id="side3SweepY" class="setting-input" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE3SWEEPY', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Shift Speed</h3>
                        <input type="number" id="side3PaintingXSpeed" class="setting-input" max="20000" step="1000" placeholder="20" onchange="updatePatternSetting('SIDE3PAINTINGXSPEED', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Sweep Speed</h3>
                        <input type="number" id="side3PaintingYSpeed" class="setting-input" max="20000" step="1000" placeholder="20" onchange="updatePatternSetting('SIDE3PAINTINGYSPEED', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Z Height</h3>
                        <input type="number" id="side3ZHeight" class="setting-input" min="-2.5" max="0" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE3ZHEIGHT', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Servo Angle</h3>
                        <input type="number" id="servoAngleSide3" class="setting-input" min="30" max="100" step="1" placeholder="0" onchange="updatePatternSetting('SERVO_ANGLE_SIDE3', this.value)">
                    </div>
                </div>
            </div>
            
            <!-- Side 1 Settings Tab Content -->
            <div class="pattern-tab-content" id="side1Tab">
                <div class="pattern-settings-grid">
                    <div class="pattern-setting-group">
                        <h3>Side 1 Starting Position</h3>
                        <div class="setting-inputs labeled-inputs">
                            <label for="side1StartX">X:</label>
                            <input type="number" id="side1StartX" class="setting-input" min="0" max="37" placeholder="0.0" onchange="updatePatternSetting('SIDE1STARTX', this.value)">
                            <label for="side1StartY">Y:</label>
                            <input type="number" id="side1StartY" class="setting-input" min="0" max="37" placeholder="0.0" onchange="updatePatternSetting('SIDE1STARTY', this.value)">
                        </div>
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Shift Distance</h3>
                        <input type="number" id="side1ShiftX" class="setting-input" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE1SHIFTX', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Sweep Distance</h3>
                        <input type="number" id="side1SweepY" class="setting-input" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE1SWEEPY', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Shift Speed</h3>
                        <input type="number" id="side1PaintingXSpeed" class="setting-input" max="20000" step="1000" placeholder="20" onchange="updatePatternSetting('SIDE1PAINTINGXSPEED', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Sweep Speed</h3>
                        <input type="number" id="side1PaintingYSpeed" class="setting-input" max="20000" step="1000" placeholder="20" onchange="updatePatternSetting('SIDE1PAINTINGYSPEED', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Z Height</h3>
                        <input type="number" id="side1ZHeight" class="setting-input" min="-2.5" max="0" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE1ZHEIGHT', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Servo Angle</h3>
                        <input type="number" id="servoAngleSide1" class="setting-input" min="30" max="100" step="1" placeholder="0" onchange="updatePatternSetting('SERVO_ANGLE_SIDE1', this.value)">
                    </div>
                </div>
            </div>
            
            <!-- Side 4 Settings Tab Content -->
            <div class="pattern-tab-content" id="side4Tab">
                <div class="pattern-settings-grid">
                    <div class="pattern-setting-group">
                        <h3>Side 4 Starting Position</h3>
                        <div class="setting-inputs labeled-inputs">
                            <label for="side4StartX">X:</label>
                            <input type="number" id="side4StartX" class="setting-input" min="0" max="37" placeholder="0.0" onchange="updatePatternSetting('SIDE4STARTX', this.value)">
                            <label for="side4StartY">Y:</label>
                            <input type="number" id="side4StartY" class="setting-input" min="0" max="37" placeholder="0.0" onchange="updatePatternSetting('SIDE4STARTY', this.value)">
                        </div>
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Shift Distance</h3>
                        <input type="number" id="side4ShiftX" class="setting-input" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE4SHIFTX', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Sweep Distance</h3>
                        <input type="number" id="side4SweepY" class="setting-input" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE4SWEEPY', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Shift Speed</h3>
                        <input type="number" id="side4PaintingXSpeed" class="setting-input" max="20000" step="1000" placeholder="20" onchange="updatePatternSetting('SIDE4PAINTINGXSPEED', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Sweep Speed</h3>
                        <input type="number" id="side4PaintingYSpeed" class="setting-input" max="20000" step="1000" placeholder="20" onchange="updatePatternSetting('SIDE4PAINTINGYSPEED', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Z Height</h3>
                        <input type="number" id="side4ZHeight" class="setting-input" min="-2.5" max="0" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE4ZHEIGHT', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Servo Angle</h3>
                        <input type="number" id="servoAngleSide4" class="setting-input" min="30" max="100" step="1" placeholder="0" onchange="updatePatternSetting('SERVO_ANGLE_SIDE4', this.value)">
                    </div>
                </div>
            </div>
            
            <!-- Side 2 Settings Tab Content -->
            <div class="pattern-tab-content" id="side2Tab">
                <div class="pattern-settings-grid">
                    <div class="pattern-setting-group">
                        <h3>Side 2 Starting Position</h3>
                        <div class="setting-inputs labeled-inputs">
                            <label for="side2StartX">X:</label>
                            <input type="number" id="side2StartX" class="setting-input" min="0" max="37" placeholder="0.0" onchange="updatePatternSetting('SIDE2STARTX', this.value)">
                            <label for="side2StartY">Y:</label>
                            <input type="number" id="side2StartY" class="setting-input" min="0" max="37" placeholder="0.0" onchange="updatePatternSetting('SIDE2STARTY', this.value)">
                        </div>
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Shift Distance</h3>
                        <input type="number" id="side2ShiftX" class="setting-input" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE2SHIFTX', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Sweep Distance</h3>
                        <input type="number" id="side2SweepY" class="setting-input" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE2SWEEPY', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Shift Speed</h3>
                        <input type="number" id="side2PaintingXSpeed" class="setting-input" max="20000" step="1000" placeholder="20" onchange="updatePatternSetting('SIDE2PAINTINGXSPEED', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Sweep Speed</h3>
                        <input type="number" id="side2PaintingYSpeed" class="setting-input" max="20000" step="1000" placeholder="20" onchange="updatePatternSetting('SIDE2PAINTINGYSPEED', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Z Height</h3>
                        <input type="number" id="side2ZHeight" class="setting-input" min="-2.5" max="0" step="0.1" placeholder="0.0" onchange="updatePatternSetting('SIDE2ZHEIGHT', this.value)">
                    </div>
                    <div class="pattern-setting-group">
                        <h3>Servo Angle</h3>
                        <input type="number" id="servoAngleSide2" class="setting-input" min="30" max="100" step="1" placeholder="0" onchange="updatePatternSetting('SERVO_ANGLE_SIDE2', this.value)">
                    </div>
                </div>
            </div>
            
            <div class="main-divider"></div>
            
            <!-- Settings Controls -->
            <div class="settings-controls">
                <!-- Remove Load and Save buttons -->
                <!-- <button class="main-btn blue" onclick="loadPatternSettings()">Load Settings</button> -->
                <!-- <button class="main-btn highlight" onclick="savePatternSettings()">Save Settings</button> -->
            </div>
        </div> <!-- End of pattern-settings-content-wrapper -->
    </div>
</body>
</html>
)rawliteral";

#endif // HTML_CONTENT_H
