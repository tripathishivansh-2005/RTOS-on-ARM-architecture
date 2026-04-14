@echo off
REM build.bat — Build the RTOS project without needing "make"
REM Use this if "make" is not installed on your Windows machine

REM Ensure Toolchain and QEMU are in the path
SET PATH=%PATH%;C:\Program Files (x86)\Arm\GNU Toolchain mingw-w64-i686-arm-none-eabi\bin;C:\Program Files\qemu

echo.
echo ====================================================
echo  Building ARM Cortex-M RTOS Project
echo ====================================================
echo.

REM Create build directory
if not exist "build" mkdir build
if not exist "build\src"      mkdir build\src
if not exist "build\startup"  mkdir build\startup
if not exist "build\freertos" mkdir build\freertos
if not exist "build\freertos\portable\GCC\ARM_CM3" mkdir build\freertos\portable\GCC\ARM_CM3
if not exist "build\freertos\portable\MemMang"     mkdir build\freertos\portable\MemMang

SET CC=arm-none-eabi-gcc
SET CFLAGS=-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -Os -g -Wall --specs=nosys.specs -DQEMU_BUILD=1
SET INC=-Iinclude -Ifreertos\include -Ifreertos\portable\GCC\ARM_CM3
SET ASFLAGS=-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -g
SET LDFLAGS=-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -Tld/mps2.ld -Wl,-Map=build/rtos_demo.map -Wl,--gc-sections --specs=nosys.specs -lc -lm

echo [CC] Compiling startup...
%CC% %CFLAGS% %INC% -c startup\startup.c -o build\startup\startup.o
if errorlevel 1 goto :error

echo [CC] Compiling syscalls...
%CC% %CFLAGS% %INC% -c src\syscalls.c -o build\src\syscalls.o
if errorlevel 1 goto :error

echo [CC] Compiling main...
%CC% %CFLAGS% %INC% -c src\main.c -o build\src\main.o
if errorlevel 1 goto :error

echo [CC] Compiling Mayank tasks...
%CC% %CFLAGS% %INC% -c src\mayank_tasks.c -o build\src\mayank_tasks.o
if errorlevel 1 goto :error

echo [CC] Compiling Deepanshu tasks...
%CC% %CFLAGS% %INC% -c src\deepanshu_tasks.c -o build\src\deepanshu_tasks.o
if errorlevel 1 goto :error

echo [AS] Assembling asm_add.s (Thumb-2)...
%CC% %ASFLAGS% %INC% -c src\asm_add.s -o build\src\asm_add.o
if errorlevel 1 goto :error

echo [CC] Compiling Shivansh tasks...
%CC% %CFLAGS% %INC% -c src\shivansh_tasks.c -o build\src\shivansh_tasks.o
if errorlevel 1 goto :error

echo [CC] Compiling Yash tasks...
%CC% %CFLAGS% %INC% -c src\yash_tasks.c -o build\src\yash_tasks.o
if errorlevel 1 goto :error

echo [CC] Compiling LCD driver...
%CC% %CFLAGS% %INC% -c src\lcd.c -o build\src\lcd.o
if errorlevel 1 goto :error

echo [CC] Compiling Visual task...
%CC% %CFLAGS% %INC% -c src\visual_task.c -o build\src\visual_task.o
if errorlevel 1 goto :error

echo [CC] Compiling FreeRTOS kernel...
%CC% %CFLAGS% %INC% -c freertos\tasks.c        -o build\freertos\tasks.o
%CC% %CFLAGS% %INC% -c freertos\queue.c        -o build\freertos\queue.o
%CC% %CFLAGS% %INC% -c freertos\list.c         -o build\freertos\list.o
%CC% %CFLAGS% %INC% -c freertos\timers.c       -o build\freertos\timers.o
%CC% %CFLAGS% %INC% -c freertos\event_groups.c -o build\freertos\event_groups.o
%CC% %CFLAGS% %INC% -c freertos\portable\GCC\ARM_CM3\port.c -o build\freertos\portable\GCC\ARM_CM3\port.o
%CC% %CFLAGS% %INC% -c freertos\portable\MemMang\heap_4.c   -o build\freertos\portable\MemMang\heap_4.o
if errorlevel 1 goto :error

echo [LD] Linking...
%CC% %LDFLAGS% ^
    build\startup\startup.o ^
    build\src\syscalls.o ^
    build\src\main.o ^
    build\src\mayank_tasks.o ^
    build\src\deepanshu_tasks.o ^
    build\src\asm_add.o ^
    build\src\shivansh_tasks.o ^
    build\src\yash_tasks.o ^
    build\src\lcd.o ^
    build\src\visual_task.o ^
    build\freertos\tasks.o ^
    build\freertos\queue.o ^
    build\freertos\list.o ^
    build\freertos\timers.o ^
    build\freertos\event_groups.o ^
    build\freertos\portable\GCC\ARM_CM3\port.o ^
    build\freertos\portable\MemMang\heap_4.o ^
    -o build\rtos_demo.elf
if errorlevel 1 goto :error

echo.
echo ====================================================
echo  BUILD SUCCESSFUL!
echo  Run: run_qemu.bat   OR   make run
echo ====================================================
echo.
arm-none-eabi-size build\rtos_demo.elf
goto :end

:error
echo.
echo BUILD FAILED! See error above.
echo Check README.txt for troubleshooting.
pause
exit /b 1

:end
pause
