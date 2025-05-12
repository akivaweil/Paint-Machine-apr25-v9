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
        <h2>Status</h2>
// ... existing code ...
