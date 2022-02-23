@echo off
cd "E:\Projects\MicroPython\CMIOT-PY\user\port\windows"
Setlocal Enabledelayedexpansion

echo -------------------------start compiling CMIOT-PY at %date% %time% -------------------------
echo 建立日期 %date% %time% > build.log
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\Common7\IDE\devenv.exe" micropython.sln /build RELEASE /out output.log
type output.log
type output.log >> build.log
del output.log
echo -------------------------Finish compiling CMIOT-PY at %date% %time% -------------------------
call micropython.exe
