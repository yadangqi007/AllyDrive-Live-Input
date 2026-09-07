# AllyDrive Live Input 0.4.0 — English guide

## What it changes

AllyDrive processes the left stick before Automobilista 2 receives it. Steering remains direct at parking speed, then becomes progressively calmer as speed rises. Countersteer is limited according to the measured slide instead of applying full opposite lock immediately.

The chase-camera layer adds smooth yaw lag, a small acceleration/braking pitch effect, automatic chase-view selection and an automatic FOV target of 55. Manual right-stick input always takes priority.

## Installation

1. Close Automobilista 2.
2. Download and fully extract `AllyDrive-Live-Input-0.4.0.zip`.
3. Run `Instalar-AllyDrive.exe`.
4. Accept the Windows administrator prompt.
5. Wait for the **Installation completed** message.
6. Start AMS2 and remain stopped for about five seconds after entering the track while the camera initializes.

The installer places `XINPUT9_1_0.dll` and `allydrive.ini` beside `AMS2AVX.exe`. If it finds an unrelated local XInput proxy, it creates a backup before replacing it.

## Required AMS2 setting

Set **Options → System → Shared Memory** to **Project CARS 2**. This is required once. Without telemetry, basic stick shaping remains active, but speed adaptation, slide detection and dynamic camera movement are unavailable.

Recommended neutral controller baseline:

- Steering Deadzone: `0`
- Steering Sensitivity: `50`
- Speed Sensitivity: `0`
- Controller Damping: `0`

These values prevent AMS2 from applying a second strong filter after AllyDrive.

## Controls

- Hold **LB + RB + View** to toggle steering processing at runtime.
- Move the right stick to override the automatic chase-camera look.

## Uninstallation

Close AMS2 and run `DESINSTALAR.cmd` as administrator. It removes AllyDrive and restores any proxy that the installer backed up.

## Configuration

Advanced users can edit `allydrive.ini` while the game is closed. The `[Filter]` section controls steering behavior and `[Camera]` controls chase-camera motion and automatic FOV setup.

## Known limitations

- The installer currently targets the default Steam folder: `C:\Program Files (x86)\Steam\steamapps\common\Automobilista 2`.
- The executable and DLL are unsigned, so Windows may display a publisher warning.
- The camera uses AMS2 input and telemetry interfaces; it cannot replace the engine's internal camera implementation exactly.
- Test offline first.
