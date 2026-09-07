# AllyDrive Live Input 0.4.0 — Guía en español

## Qué modifica

AllyDrive procesa el stick izquierdo antes de que Automobilista 2 reciba la señal. La dirección conserva suficiente giro a velocidad de estacionamiento y se vuelve progresivamente más tranquila al aumentar la velocidad. El contravolante se limita según el derrape medido, en lugar de aplicar inmediatamente todo el bloqueo contrario.

La cámara chase añade retraso suave frente a la guiñada, un movimiento pequeño al acelerar o frenar, selección automática de la vista chase y FOV automático de 55. El movimiento manual del stick derecho siempre tiene prioridad.

## Instalación

1. Cierra Automobilista 2.
2. Descarga y extrae completamente `AllyDrive-Live-Input-0.4.0.zip`.
3. Ejecuta `Instalar-AllyDrive.exe`.
4. Acepta el permiso de administrador de Windows.
5. Espera el mensaje **Instalación completada**.
6. Inicia AMS2 y permanece detenido unos cinco segundos al entrar a pista mientras se prepara la cámara.

El instalador coloca `XINPUT9_1_0.dll` y `allydrive.ini` junto a `AMS2AVX.exe`. Si encuentra otro proxy XInput local que no pertenece a AllyDrive, crea un respaldo antes de sustituirlo.

## Ajuste necesario de AMS2

Selecciona una vez **Options → System → Shared Memory → Project CARS 2**. Sin telemetría seguirá funcionando la curva básica del stick, pero no estarán disponibles la adaptación por velocidad, detección de derrape ni cámara dinámica.

Valores neutrales recomendados para evitar un segundo filtrado:

- Steering Deadzone: `0`
- Steering Sensitivity: `50`
- Speed Sensitivity: `0`
- Controller Damping: `0`

## Controles

- Mantén **LB + RB + View** para activar o desactivar el procesamiento de dirección.
- Mueve el stick derecho para anular temporalmente el movimiento automático de la cámara.

## Desinstalación

Cierra AMS2 y ejecuta `DESINSTALAR.cmd` como administrador. Elimina AllyDrive y restaura cualquier proxy respaldado por el instalador.

## Configuración avanzada

Puedes editar `allydrive.ini` con el juego cerrado. La sección `[Filter]` controla la dirección y `[Camera]` controla el movimiento chase y la preparación automática del FOV.

## Limitaciones conocidas

- El instalador actual busca la ruta predeterminada de Steam: `C:\Program Files (x86)\Steam\steamapps\common\Automobilista 2`.
- El ejecutable y el DLL no están firmados; Windows puede mostrar un aviso de editor desconocido.
- La cámara usa las interfaces de entrada y telemetría de AMS2; no puede sustituir exactamente la cámara interna del motor.
- Realiza las primeras pruebas sin conexión.
