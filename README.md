# AllyDrive Live Input for Automobilista 2

[English instructions](README_EN.md) · [Instrucciones en español](README_ES.md)

AllyDrive is a live controller-processing layer for **Automobilista 2**. It intercepts the left-stick steering signal through a local XInput proxy, reads the game's official shared-memory telemetry, and adjusts steering response according to vehicle speed, yaw, lateral slip and recovery state.

AllyDrive also includes an experimental telemetry-driven chase camera with automatic chase-view selection and FOV setup.

## Version 0.4.0 highlights

- Speed-adaptive steering response designed for gamepad play.
- Progressive steering reduction from low speed through 200 km/h.
- Slip/yaw-proportional countersteer with controlled centre crossing.
- Continuous stability blending to prevent low-speed oscillation.
- Dynamic chase-camera yaw lag and acceleration pitch.
- Automatic chase-camera selection and FOV 55 setup when entering the track.
- One-click Windows installer and reversible uninstall procedure.

## Download

Download `AllyDrive-Live-Input-0.4.0.zip` from the latest GitHub Release. Close AMS2, extract the ZIP and run `Instalar-AllyDrive.exe` as administrator.

## Compatibility

- Automobilista 2 for Windows, Steam default installation path.
- XInput-compatible controller.
- AMS2 **Shared Memory: Project CARS 2** is required for speed, yaw, slip and camera adaptation.

## Important scope

AllyDrive approximates several controller and camera principles documented for Forza Motorsport. It does not contain Turn 10 code and cannot reproduce Forza's internal vehicle-dynamics or yaw-inertia changes.

Use the first tests in offline sessions. Game updates may replace or invalidate local proxy files.

## Source and validation

The repository includes the XInput proxy, filter core, installer source and validation programs. Version 0.4.0 passed:

- steering and countersteer behavior tests;
- 60 Hz versus 240 Hz time-integration checks;
- AMS2 shared-memory offset verification against the installed official SDK;
- XInput proxy loading, forwarding and export checks.

See [CHANGELOG.md](CHANGELOG.md) for the complete history.
