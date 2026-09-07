# AllyDrive Live Input 0.4.0

- Replaces the binary slide trigger with a continuous instability blend.
- Raises the permitted yaw threshold in slow corners and disables stability intervention below 45 km/h, eliminating low-speed steering oscillation.
- Adds telemetry-driven chase-camera yaw lag and acceleration pitch while preserving manual right-stick control.
- Automatically selects chase view and normalizes its FOV to 55 when entering the track, without opening settings menus.

## 0.3.0

- Replaces fixed, very fast countersteer with yaw/slip-proportional opposite lock.
- Slows the first crossing through centre to prevent snap oversteer.
- Preserves a faster correction rate once steering is established on the opposite side.
- Removes stale countersteer sooner when yaw reverses, reducing the pendulum effect.

## 0.2.0

- Moves the steering reduction much earlier to correct excessive sensitivity below 200 km/h.
- Replaces the late smoothstep envelope with a continuous exponential speed curve.
- Approximate maximum steering is now 64% at 50 km/h, 44% at 100 km/h, 36% at 150 km/h and 32% at 200 km/h.
- Adds more centre precision and slightly slower turn-in without reducing countersteer speed.

## 0.1.0

- Replaces the settings-only approach with a live XInput steering processor.
- Adds speed-adaptive steering lock and turn-in rate.
- Adds independent recentering and countersteer response.
- Uses AMS2 yaw and lateral-velocity telemetry to soften destabilizing input.
- Integrates at fixed 240 Hz so frame-rate changes do not alter steering feel.
- Includes an instant **LB + RB + View** comparison toggle.
