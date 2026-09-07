# AllyDrive Live Input v0.4.0

## English

This release turns AllyDrive into a more complete gamepad layer for Automobilista 2. Steering sensitivity now decreases progressively well before 200 km/h, countersteer is proportional to measured yaw and slip, and slow corners no longer trigger the stability correction repeatedly.

The new experimental chase camera adds smooth rotational lag and subtle acceleration/braking movement. It automatically selects chase view and sets FOV 55 when entering the track; manual right-stick input remains available at all times.

### Installation

Close AMS2, extract the ZIP and run `Instalar-AllyDrive.exe` as administrator. Enable **Shared Memory → Project CARS 2** once in AMS2. Remain stopped for about five seconds when entering the track so the camera can initialize.

### Validation

Tested for XInput forwarding, official AMS2 shared-memory layout, fixed-time response at 60/240 Hz, progressive speed limiting, countersteer recovery and low-speed stability.

## Español

Esta versión convierte AllyDrive en una capa de control más completa para Automobilista 2. La sensibilidad disminuye progresivamente mucho antes de 200 km/h, el contravolante responde en proporción a la guiñada y el derrape medidos, y las curvas lentas dejan de activar repetidamente la corrección de estabilidad.

La nueva cámara chase experimental añade retraso rotacional suave y movimiento sutil al acelerar o frenar. Selecciona automáticamente la vista chase y configura FOV 55 al entrar a pista; el stick derecho permanece disponible en todo momento.

### Instalación

Cierra AMS2, extrae el ZIP y ejecuta `Instalar-AllyDrive.exe` como administrador. Activa una vez **Shared Memory → Project CARS 2** en AMS2. Permanece detenido unos cinco segundos al entrar a pista para que la cámara se inicialice.

### Validación

Probado con reenvío XInput, estructura oficial de memoria compartida de AMS2, respuesta temporal fija a 60/240 Hz, límite progresivo por velocidad, recuperación del contravolante y estabilidad a baja velocidad.
