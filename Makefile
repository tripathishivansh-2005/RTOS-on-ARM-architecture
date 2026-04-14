# Makefile — ARM Cortex-M RTOS on QEMU MPS2-AN385
#
# Usage:
#   make          — build the project
#   make clean    — remove build files
#   make run      — build + run on QEMU
#   make debug    — build + run with GDB server (port 1234)
#
# Windows: use this Makefile with "make" from Git Bash or MSYS2
# Linux/Mac: works directly

# ── Tools ────────────────────────────────────────────────
CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc   # Use GCC as assembler (handles .s files)
LD      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

QEMU    = qemu-system-arm

# ── Target ───────────────────────────────────────────────
TARGET  = rtos_demo
BUILD   = build

# ── Compiler flags ───────────────────────────────────────
# -mcpu=cortex-m3  : generate Cortex-M3 instructions
# -mthumb          : Thumb-2 instruction set
# -mfloat-abi=soft : no FPU (MPS2-AN385 is M3, no FPU)
# -Os              : optimize for size (fits in 32KB if using Keil free)
# -g               : debug symbols
# --specs=nosys.specs : use our own syscalls.c (no semi-hosting)
# -Wall -Wextra    : enable warnings

CFLAGS  = -mcpu=cortex-m3 \
           -mthumb \
           -mfloat-abi=soft \
           -Os \
           -g \
           -Wall \
           -Wextra \
           -ffunction-sections \
           -fdata-sections \
           --specs=nosys.specs \
           -DQEMU_BUILD=1

ASFLAGS = -mcpu=cortex-m3 \
           -mthumb \
           -mfloat-abi=soft \
           -g

# ── Linker flags ─────────────────────────────────────────
LDFLAGS = -mcpu=cortex-m3 \
           -mthumb \
           -mfloat-abi=soft \
           -T ld/mps2.ld \
           -Wl,-Map=$(BUILD)/$(TARGET).map \
           -Wl,--gc-sections \
           --specs=nosys.specs \
           -lc -lm

# ── Include paths ─────────────────────────────────────────
INCLUDES = \
    -I include \
    -I freertos/include \
    -I freertos/portable/GCC/ARM_CM3

# ── Source files ──────────────────────────────────────────
# Our project sources
APP_C_SRCS = \
    src/main.c \
    src/syscalls.c \
    src/mayank_tasks.c \
    src/deepanshu_tasks.c \
    src/shivansh_tasks.c \
    src/yash_tasks.c \
    src/lcd.c \
    src/visual_task.c

# Assembly sources (Deepanshu's ISA demo)
APP_S_SRCS = \
    src/asm_add.s

# Startup
STARTUP_C = \
    startup/startup.c

# FreeRTOS kernel sources
FREERTOS_SRCS = \
    freertos/tasks.c \
    freertos/queue.c \
    freertos/list.c \
    freertos/timers.c \
    freertos/event_groups.c \
    freertos/portable/GCC/ARM_CM3/port.c \
    freertos/portable/MemMang/heap_4.c

# ── All sources combined ───────────────────────────────────
ALL_C_SRCS  = $(APP_C_SRCS) $(STARTUP_C) $(FREERTOS_SRCS)
ALL_S_SRCS  = $(APP_S_SRCS)

# ── Object files ──────────────────────────────────────────
C_OBJS  = $(patsubst %.c, $(BUILD)/%.o, $(ALL_C_SRCS))
S_OBJS  = $(patsubst %.s, $(BUILD)/%.o, $(ALL_S_SRCS))
ALL_OBJS = $(C_OBJS) $(S_OBJS)

# ── Build rules ───────────────────────────────────────────

.PHONY: all clean run debug size

all: $(BUILD)/$(TARGET).elf
	@echo ""
	@echo "╔══════════════════════════════════════╗"
	@echo "║  BUILD SUCCESSFUL!                   ║"
	@echo "║  Run: make run                       ║"
	@echo "╚══════════════════════════════════════╝"
	@$(SIZE) $<

# Link
$(BUILD)/$(TARGET).elf: $(ALL_OBJS)
	@echo "[LD]  Linking $@"
	@mkdir -p $(BUILD)
	$(LD) $(LDFLAGS) $(ALL_OBJS) -o $@

# Compile .c files
$(BUILD)/%.o: %.c
	@echo "[CC]  $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Assemble .s files
$(BUILD)/%.o: %.s
	@echo "[AS]  $<"
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $(INCLUDES) -c $< -o $@

# ── Run on QEMU ───────────────────────────────────────────
run: all
	@echo ""
	@echo "Starting QEMU MPS2-AN385 Cortex-M3..."
	@echo "Press Ctrl+A then X to quit QEMU"
	@echo ""
	$(QEMU) \
	    -machine mps2-an385 \
	    -cpu cortex-m3 \
	    -kernel $(BUILD)/$(TARGET).elf \
	    -serial stdio \
	    -semihosting-config enable=on,target=native

# ── Debug with GDB ────────────────────────────────────────
debug: all
	@echo ""
	@echo "QEMU waiting for GDB on port 1234..."
	@echo "Connect with: arm-none-eabi-gdb build/rtos_demo.elf"
	@echo "GDB command:  target remote :1234"
	@echo ""
	$(QEMU) \
	    -machine mps2-an385 \
	    -cpu cortex-m3 \
	    -kernel $(BUILD)/$(TARGET).elf \
	    -serial stdio \
	    -semihosting-config enable=on,target=native \
	    -s -S

# ── Size report ───────────────────────────────────────────
size: $(BUILD)/$(TARGET).elf
	$(SIZE) $<

# ── Clean ────────────────────────────────────────────────
clean:
	@echo "[CLN] Removing build directory"
	@rm -rf $(BUILD)
	@echo "Clean done."
