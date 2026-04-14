@echo off
REM run_qemu.bat — Double-click this to run the RTOS on QEMU
REM Works on Windows CMD directly

echo.
echo ====================================================
echo  ARM Cortex-M RTOS — QEMU MPS2-AN385 Cortex-M3
echo  Press Ctrl+A then X to quit
echo ====================================================
echo.

REM Check if .elf exists
IF NOT EXIST "build\rtos_demo.elf" (
    echo ERROR: build\rtos_demo.elf not found!
    echo Run "make" first to build the project.
    echo.
    pause
    exit /b 1
)

REM Run QEMU
qemu-system-arm ^
    -machine mps2-an385 ^
    -cpu cortex-m3 ^
    -kernel build\rtos_demo.elf ^
    -serial stdio ^
    -semihosting-config enable=on,target=native

pause
