@echo off
setlocal
set "GAME=C:\Program Files (x86)\Steam\steamapps\common\Automobilista 2"

tasklist /FI "IMAGENAME eq AMS2AVX.exe" 2>NUL | find /I "AMS2AVX.exe" >NUL
if not errorlevel 1 (
  echo Cierra Automobilista 2 antes de desinstalar AllyDrive.
  pause
  exit /b 1
)

fltmc >NUL 2>&1
if not "%errorlevel%"=="0" (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
  exit /b
)

del /Q "%GAME%\XINPUT9_1_0.dll" 2>NUL
del /Q "%GAME%\allydrive.ini" 2>NUL
if exist "%GAME%\XINPUT9_1_0.dll.allydrive-backup" move /Y "%GAME%\XINPUT9_1_0.dll.allydrive-backup" "%GAME%\XINPUT9_1_0.dll" >NUL
if exist "%GAME%\allydrive.ini.allydrive-backup" move /Y "%GAME%\allydrive.ini.allydrive-backup" "%GAME%\allydrive.ini" >NUL
echo AllyDrive Live Input se desinstalo correctamente.
pause
