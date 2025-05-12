// ... existing code ...
    <div class="section">
        <h2>System Control</h2>
        <button class="btn" onclick="sendCommand('HOME_ALL')">Home All Axes</button>
        <button class="btn" onclick="sendCommand('START_PNP')">Start PnP</button>
        <!-- Add other system controls as needed -->
    </div>

    <div class="section">
        <h2>Settings Management</h2>
        <button class="btn" onclick="window.location.href='/downloadSettings'">Download Settings</button>
        <form action="/uploadSettings" method="post" enctype="multipart/form-data" style="display: inline-block; margin-left: 10px;">
            <input type="file" name="settingsFile" accept=".json" required>
            <button type="submit" class="btn">Upload Settings</button>
        </form>
    </div>

    <div class="section">
        <h2>PNP Motion Settings</h2>
        <label for="pnp_x_speed">PNP X Speed (steps/s):</label>
        <input type="number" id="pnp_x_speed" name="pnp_x_speed"><br>
        <label for="pnp_x_accel">PNP X Accel (steps/s²):</label>
        <input type="number" id="pnp_x_accel" name="pnp_x_accel"><br>
        <label for="pnp_y_speed">PNP Y Speed (steps/s):</label>
        <input type="number" id="pnp_y_speed" name="pnp_y_speed"><br>
        <label for="pnp_y_accel">PNP Y Accel (steps/s²):</label>
        <input type="number" id="pnp_y_accel" name="pnp_y_accel"><br>
        <button class="btn" onclick="updatePnpSettings()">Update PNP Settings</button>
    </div>

    <div class="section">
        <h2>Status</h2>
// ... existing code ...

    function sendCommand(command) {
        websocket.send(JSON.stringify({ command: command }));
    }

    function updatePnpSettings() {
        const pnpXSpeed = document.getElementById('pnp_x_speed').value;
        const pnpXAccel = document.getElementById('pnp_x_accel').value;
        const pnpYSpeed = document.getElementById('pnp_y_speed').value;
        const pnpYAccel = document.getElementById('pnp_y_accel').value;
        
        const payload = {
            command: "update_pnp_settings",
            pnp_x_speed: parseInt(pnpXSpeed),
            pnp_x_accel: parseInt(pnpXAccel),
            pnp_y_speed: parseInt(pnpYSpeed),
            pnp_y_accel: parseInt(pnpYAccel)
        };
        websocket.send(JSON.stringify(payload));
        // Optionally, provide feedback to the user, e.g., alert("PNP settings sent!");
    }

    websocket.onopen = function(event) {
        console.log("WebSocket connection opened");
        // Request current PNP settings when connection opens
        websocket.send(JSON.stringify({ command: "GET_PNP_SETTINGS" }));
        // You might want to request other initial settings here too, e.g., GET_PAINT_SETTINGS
    };

    websocket.onmessage = function(event) {
        console.log("Message from server: ", event.data);
        try {
            const data = JSON.parse(event.data);
            if (data.event === "pnp_settings") {
                if (data.pnp_x_speed !== undefined) document.getElementById('pnp_x_speed').value = data.pnp_x_speed;
                if (data.pnp_x_accel !== undefined) document.getElementById('pnp_x_accel').value = data.pnp_x_accel;
                if (data.pnp_y_speed !== undefined) document.getElementById('pnp_y_speed').value = data.pnp_y_speed;
                if (data.pnp_y_accel !== undefined) document.getElementById('pnp_y_accel').value = data.pnp_y_accel;
                console.log("PNP settings populated in UI.");
            } else if (event.data.startsWith("STATE:")) {
                document.getElementById('machineState').textContent = event.data.substring(6);
            } else if (event.data.startsWith("PAINT_GUN_STATUS:")) {
                // Update paint gun toggle or status display if you have one
                console.log("Paint gun status: ", event.data.substring(19));
            } else if (event.data.startsWith("PRESSURE_POT_STATUS:")) {
                // Update pressure pot toggle or status display
                console.log("Pressure pot status: ", event.data.substring(22));
            } else {
                // Handle other text messages (like CMD_ACK, CMD_ERROR, STATUS etc.)
                // Potentially display these in a log area on the page
                 document.getElementById('statusMessages').textContent += event.data + '\n'; // Append to a log
            }
        } catch (e) {
            // If it's not JSON, treat as plain text status/log message
            console.log("Received non-JSON message or error parsing: ", event.data);
            if (event.data.startsWith("STATE:")) { // Ensure state updates are caught even if try block fails early
                document.getElementById('machineState').textContent = event.data.substring(6);
            } else {
                 document.getElementById('statusMessages').textContent += event.data + '\n'; // Append to a log
            }
        }
    };

    websocket.onclose = function(event) {
// ... existing code ...
