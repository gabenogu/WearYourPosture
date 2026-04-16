# Wear Your Posture Frontend

This folder contains the React dashboard for the device telemetry UI.

## Requirements

- Node.js 20 or newer
- npm
- A browser

For live serial telemetry, use a Chromium-based browser such as Chrome or Edge because the dashboard uses the Web Serial API.

## Open The Website Locally

1. Open a terminal in this folder:

```powershell
cd c:\Users\Gabriel Rodriguez\Downloads\WearYourPosture\frontend
```

2. Install dependencies:

```powershell
npm install
```

3. Start the dev server:

```powershell
npm run dev -- --host 127.0.0.1 --port 4173
```

4. Open this URL in your browser:

```text
http://127.0.0.1:4173/
```

5. When you are done, stop the server with `Ctrl+C` in that terminal.

## Open The Website On Another Device On Your Network

If you want to open the dashboard from another phone or computer on the same Wi-Fi network, run:

```powershell
npm run dev -- --host 0.0.0.0 --port 4173
```

Then open:

```text
http://YOUR_COMPUTER_IP:4173/
```

Example from this machine:

```text
http://192.168.4.73:4173/
```

## Use The Dashboard

- `Demo Stream` shows simulated device data so the UI always has something to display.
- `Live Serial` connects to the ESP32 over USB serial and reads `WYP_TELEMETRY` messages from the firmware.
- BLE advertising already exists in the firmware, but browser BLE streaming is not wired up yet.

## Build A Production Bundle

To create a production build:

```powershell
npm run build
```

The built files will be written to the `dist/` folder.
