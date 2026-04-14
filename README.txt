╔══════════════════════════════════════════════════════════════════╗
║  ARM Cortex-M RTOS on QEMU — Complete Project                   ║
║  Team: Mayank | Deepanshu | Shivansh | Yash                     ║
║  Target: QEMU MPS2-AN385 (Cortex-M3, 25MHz)                    ║
╚══════════════════════════════════════════════════════════════════╝


━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
STEP 0 — WHAT IS ALREADY DONE
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
This ZIP contains ALL your project files already written:

  ld/mps2.ld              — Linker script (Flash + SRAM memory map)
  startup/startup.c       — Reset_Handler, vector table, boot sequence
  include/FreeRTOSConfig.h— FreeRTOS configuration for QEMU
  include/uart.h          — MPS2 UART0 driver (polling)
  src/syscalls.c          — printf → UART redirect
  src/main.c              — All 11 tasks created, scheduler start
  src/mayank_tasks.c      — LED_High, LED_Low, Probe tasks
  src/deepanshu_tasks.c   — ISA_High, ISA_Medium, ISA_Low tasks
  src/asm_add.s           — Thumb-2 assembly: asm_add, asm_mul, asm_and
  src/shivansh_tasks.c    — Sensor→Queue_A→Process→Queue_B→Display
  src/yash_tasks.c        — Heartbeat, OS_Info, exception/SysTick demo
  Makefile                — Build system (make / make run)

YOU ONLY NEED TO: Add FreeRTOS kernel files (Step 2 below)


━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
STEP 1 — TOOLS CHECK (already installed per your setup)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Open CMD and verify:

    arm-none-eabi-gcc --version
    qemu-system-arm --version
    make --version

All three should print version info. If "make" fails on Windows:
→ Install from: gnuwin32.sourceforge.net/packages/make.htm
  OR use Git Bash (comes with make built in)
→ Add make.exe to PATH


━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
STEP 2 — ADD FREERTOS KERNEL (10 minutes, do once)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

2a. Download FreeRTOS kernel ZIP:
    https://github.com/FreeRTOS/FreeRTOS-Kernel/releases/latest
    → Click "Source code (zip)" — download FreeRTOS-Kernel-x.x.x.zip

2b. Extract the ZIP. You will get folder: FreeRTOS-Kernel-x.x.x/

2c. Copy these files INTO the project's freertos/ folder:

    FROM: FreeRTOS-Kernel-x.x.x/
    COPY: tasks.c          → freertos/tasks.c
    COPY: queue.c          → freertos/queue.c
    COPY: list.c           → freertos/list.c
    COPY: timers.c         → freertos/timers.c
    COPY: event_groups.c   → freertos/event_groups.c

    FROM: FreeRTOS-Kernel-x.x.x/include/
    COPY: ALL .h files     → freertos/include/
    (FreeRTOS.h, task.h, queue.h, semphr.h, timers.h,
     list.h, event_groups.h, portable.h, projdefs.h,
     portmacro.h, croutine.h, message_buffer.h, etc.)

    FROM: FreeRTOS-Kernel-x.x.x/portable/GCC/ARM_CM3/
    COPY: port.c           → freertos/portable/GCC/ARM_CM3/port.c
    COPY: portmacro.h      → freertos/portable/GCC/ARM_CM3/portmacro.h

    FROM: FreeRTOS-Kernel-x.x.x/portable/MemMang/
    COPY: heap_4.c         → freertos/portable/MemMang/heap_4.c

2d. Your freertos/ folder should look like:
    freertos/
    ├── tasks.c
    ├── queue.c
    ├── list.c
    ├── timers.c
    ├── event_groups.c
    ├── include/
    │   ├── FreeRTOS.h
    │   ├── task.h
    │   ├── queue.h
    │   ├── semphr.h
    │   └── ... (all .h files)
    ├── portable/
    │   ├── GCC/ARM_CM3/
    │   │   ├── port.c
    │   │   └── portmacro.h
    │   └── MemMang/
    │       └── heap_4.c


━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
STEP 3 — BUILD THE PROJECT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Open CMD (or Git Bash) in the project folder and run:

    make

Expected output (last few lines):
    [LD]  Linking build/rtos_demo.elf
    ╔══════════════════════════════════════╗
    ║  BUILD SUCCESSFUL!                   ║
    ║  Run: make run                       ║
    ╚══════════════════════════════════════╝
    text    data     bss     dec
    xxxxx   xxxxx   xxxxx   xxxxx

If you see errors → see TROUBLESHOOTING below.


━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
STEP 4 — RUN ON QEMU
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

    make run

OR directly:

    qemu-system-arm -machine mps2-an385 -cpu cortex-m3 ^
      -kernel build/rtos_demo.elf -nographic -serial stdio ^
      -semihosting-config enable=on,target=native

You should immediately see output like:
    ╔══════════════════════════════════════════════════╗
    ║  ARM Cortex-M RTOS on QEMU MPS2-AN385           ║
    ╚══════════════════════════════════════════════════╝

    [DEEPANSHU] Reset_Handler called...
    [SHIVANSH]  Memory map: Flash 0x00000000, SRAM 0x20000000
    [YASH]      Vector table Entry[0] MSP = 0x20004000
    [YASH]      SysTick RELOAD = 24999
    [MAIN]      All 11 tasks created. Starting scheduler...

    [YASH]   HB    tick=500   beat=1  ♥
    [DEEPANSHU] HIGH  tick=200  asm_add(10,20)=30  asm_mul(6,7)=42
    [SHIVANSH] SENSOR tick=300  val=5  queued=OK
    ...

TO QUIT QEMU: Press Ctrl+A, then press X


━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
STEP 5 — DEBUG WITH GDB (OPTIONAL — for deeper understanding)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Terminal 1 — Start QEMU in debug mode:
    make debug

Terminal 2 — Connect GDB:
    arm-none-eabi-gdb build/rtos_demo.elf
    (gdb) target remote :1234
    (gdb) monitor reset
    (gdb) b main
    (gdb) continue
    (gdb) info registers        ← see R0-R15, PC, SP, xPSR
    (gdb) x/8x $sp              ← see stack contents
    (gdb) info threads          ← see all FreeRTOS tasks
    (gdb) b Task_ISA_High       ← breakpoint in Deepanshu's task
    (gdb) continue
    (gdb) p result_add          ← see asm_add() result


━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
TROUBLESHOOTING
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ERROR: "make: command not found"
→ Install GNU Make for Windows from gnuwin32.sourceforge.net
  OR use Git Bash (has make built in)

ERROR: "arm-none-eabi-gcc: command not found"
→ Download from developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
  Windows (mingw-w64-i686) version
  Install and tick "Add to PATH"

ERROR: "qemu-system-arm: command not found"
→ Add QEMU to PATH: C:\Program Files\qemu\
  Control Panel → System → Advanced → Environment Variables → PATH

ERROR: "FreeRTOS.h: No such file or directory"
→ You forgot Step 2. Copy FreeRTOS headers to freertos/include/

ERROR: "undefined reference to vPortSVCHandler"
→ Using wrong FreeRTOS port. Make sure you copied from
  portable/GCC/ARM_CM3/ (not ARM_CM4F)

ERROR: "region FLASH overflowed"
→ Code is too big. Run: make CFLAGS+=-Os
  This usually fixes it by increasing optimization.

ERROR: QEMU shows nothing / black screen
→ Make sure -serial stdio is in the command
→ Make sure uart_init() is called in main()
→ On Windows CMD, try Git Bash instead

ERROR: QEMU crashes immediately
→ Check .elf was built successfully (ls -la build/)
→ Verify: file build/rtos_demo.elf — should say ARM, 32-bit

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
EXPECTED OUTPUT SAMPLE
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

After ~5 seconds of running you should see all 4 members' output
interleaved (QEMU time is faster than real time):

[DEEPANSHU] HIGH   tick=200    asm_add(10,20)=30   asm_mul(6,7)=42  stk=210
[DEEPANSHU] HIGH   tick=400    asm_add(10,20)=30   asm_mul(6,7)=42  stk=210
[DEEPANSHU] MED    tick=500    run=1    stk=220
[SHIVANSH]  SENSOR tick=300    val=5    queued=OK   shared_ctr=1    stk=215
[MAYANK]    HIGH   tick=1000   PA5=ON   stack_free=198
[YASH]      HB     tick=500    beat=1   stk=185  ♥
[SHIVANSH]  PROCESS tick=310   raw=5    avg=1
[SHIVANSH]  DISPLAY tick=312   avg=1    stk=195
[MAYANK]    PROBE   tick=600   switches=1
[MAYANK]     >> Context switch cost: ~74 cycles = ~740ns @ 100MHz
[YASH]      OS_Info tick=2000
[YASH]       >> Tasks running = 11
[YASH]       >> Scheduler state = RUNNING

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
WHAT EACH MEMBER PROVES
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

MAYANK   — Pipeline timing (context switch ~74 cycles), 3 tasks at
           different rates proving priority scheduling (5:1 ratio)

DEEPANSHU — asm_add(10,20)=30 proves Thumb-2 ISA working on real
            CPU. Startup trace proves Reset_Handler sequence.
            asm_mul(6,7)=42 as bonus.

SHIVANSH  — Queue pipeline: Sensor value flows Sensor→Process→Display
            proving inter-task communication via SRAM circular buffer.
            Semaphore signalling + mutex shared counter proven.

YASH      — SysTick RELOAD=24999 calculation shown live.
            Vector table entries read from 0x00000000.
            EXC_RETURN values explained. 11 tasks all running.
