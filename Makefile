# --- Tool Definitions ---
CCACHE  := $(shell command -v ccache 2> /dev/null)
CC      := $(CCACHE) arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

# --- Optimization Options ---
# Default to -O0 (debug) if nothing is specified
OPT ?= debug

ifeq ($(OPT), fast)
    OPT_FLAGS := -O3
else ifeq ($(OPT), small)
    OPT_FLAGS := -Os
else
    OPT_FLAGS := -O0
endif

# --- Project Paths ---
BUILD_DIR := build
COMMON    := common
CM7       := cm7
CM4       := cm4
PCH_M7_DIR := $(BUILD_DIR)/pch/cm7
PCH_M4_DIR := $(BUILD_DIR)/pch/cm4

# --- Compilation Flags ---
# -Winvalid-pch ensures we know if the PCH isn't being used
CFLAGS_BASE := -mthumb -g $(OPT_FLAGS) -Wall -nostartfiles -Winvalid-pch \
               -I$(COMMON)/inc -I$(CM7)/inc -I$(CM4)/inc

M7_FLAGS := -mcpu=cortex-m7 -DCORE_CM7 $(CFLAGS_BASE) -I$(PCH_M7_DIR)
M4_FLAGS := -mcpu=cortex-m4 -DCORE_CM4 $(CFLAGS_BASE) -I$(PCH_M4_DIR)

MASTER_HEADER := $(COMMON)/inc/common.h

# --- Targets ---
.PHONY: all clean flash stats size

all: $(BUILD_DIR)/cm7.bin $(BUILD_DIR)/cm4.bin
	@echo "--------------------------------------"
	@$(MAKE) size
	@$(MAKE) stats

# --- PCH Generation ---
# Generates architecture-specific precompiled headers
$(PCH_M7_DIR)/common.h.gch: $(MASTER_HEADER)
	@echo "Precompiling Master Header for CM7..."
	@mkdir -p $(PCH_M7_DIR)
	@$(CC) $(M7_FLAGS) -x c-header $< -o $@

$(PCH_M4_DIR)/common.h.gch: $(MASTER_HEADER)
	@echo "Precompiling Master Header for CM4..."
	@mkdir -p $(PCH_M4_DIR)
	@$(CC) $(M4_FLAGS) -x c-header $< -o $@

# --- Core-Specific Builds ---
$(BUILD_DIR)/cm7.elf: $(CM7)/src/main.c $(COMMON)/src/utils.c $(PCH_M7_DIR)/common.h.gch
	@mkdir -p $(BUILD_DIR)
	$(CC) $(M7_FLAGS) -T $(CM7)/layout.ld $(CM7)/src/main.c $(COMMON)/src/utils.c -o $@

$(BUILD_DIR)/cm4.elf: $(CM4)/src/main.c $(COMMON)/src/utils.c $(PCH_M4_DIR)/common.h.gch
	@mkdir -p $(BUILD_DIR)
	$(CC) $(M4_FLAGS) -T $(CM4)/layout.ld $(CM4)/src/main.c $(COMMON)/src/utils.c -o $@

# --- Utilities ---
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@$(OBJCOPY) -O binary $< $@

size:
	@echo "Memory Usage (Bytes):"
	@$(SIZE) -B $(BUILD_DIR)/*.elf

stats:
	@echo "--------------------------------------"
	@ccache -s

flash: all
	openocd -f openocd.cfg \
		-c "init" \
		-c "targets" \
		-c "reset halt" \
		-c "program $(BUILD_DIR)/cm7.bin 0x08000000 verify" \
		-c "program $(BUILD_DIR)/cm4.bin 0x08100000 verify" \
		-c "reset run" \
		-c "exit"

clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Done!"
