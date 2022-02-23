@echo off
REM package
echo package beken7231N 
cd ./beken_packager
call beken_packager.exe
cp *.bin ../../../out
cd ..
REM diffpatch
call ..\..\components\net\ota\cmiot\cmiot_bin.bat "C:\Program Files\WinRAR\WinRAR.exe" oneos_config.h .\beken_packager\oneos_uart_1.00.bin
echo end package!

