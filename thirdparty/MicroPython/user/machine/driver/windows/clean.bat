@echo off
cd "E:\Projects\MicroPython\CMIOT-PY\user\port\windows"
Setlocal Enabledelayedexpansion

echo -------------------------clean CMIOT-PY VS Project %date% %time%-------------------------
del /S /Q build
del build.log
del micropython.exe
echo -------------------------Succeed to CMIOT-PY VS Project %date% %time% -------------------------
