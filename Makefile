# --- Tool Definitions ---
CCACHE  := $(shell command -v ccache 2> /dev/null)
CC      := $(CCACHE) arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

# --- Colors ---
CYAN   := \033[1;36m
YELLOW := \033[1;33m
GREEN  := \033[1;32m
NC     := \033[0m

# --- Optimization ---
OPT ?= debug
ifeq ($(OPT), fast)
    OPT_FLAGS := -O3
else ifeq ($(OPT), small)
    OPT_FLAGS := -Os
else
    OPT_FLAGS := -O0
endif

# --- Paths ---
BUILD_DIR := build
COMMON    := common
CM7       := cm7
CM4       := cm4
PCH_M7_DIR := $(BUILD_DIR)/pch/cm7
PCH_M4_DIR := $(BUILD_DIR)/pch/cm4

CFLAGS_BASE := -mthumb -g $(OPT_FLAGS) -Wall -nostartfiles -Winvalid-pch \
               -I$(COMMON)/inc -I$(CM7)/inc -I$(CM4)/inc

M7_FLAGS := -mcpu=cortex-m7 -DCORE_CM7 $(CFLAGS_BASE) -I$(PCH_M7_DIR)
M4_FLAGS := -mcpu=cortex-m4 -DCORE_CM4 $(CFLAGS_BASE) -I$(PCH_M4_DIR)

# --- Targets ---
.PHONY: all clean flash wipe debug debug-server size

all: 
	@START_TIME=$$(date +%s); \
	$(MAKE) --no-print-directory build_binaries; \
	END_TIME=$$(date +%s); \
	echo "$(GREEN)Build finished in $$((END_TIME - START_TIME)) seconds.$(NC)"

build_binaries: $(BUILD_DIR)/cm7.bin $(BUILD_DIR)/cm4.bin
	@$(SIZE) -B $(BUILD_DIR)/*.elf
	@ccache -s | grep -E "cache hit|calls" || true

$(PCH_M7_DIR)/common.h.gch: $(COMMON)/inc/common.h
	@echo "$(YELLOW)  PCH [M7]$(NC)"
	@mkdir -p $(PCH_M7_DIR)
	@$(CC) $(M7_FLAGS) -x c-header $< -o $@

$(PCH_M4_DIR)/common.h.gch: $(COMMON)/inc/common.h
	@echo "$(YELLOW)  PCH [M4]$(NC)"
	@mkdir -p $(PCH_M4_DIR)
	@$(CC) $(M4_FLAGS) -x c-header $< -o $@

$(BUILD_DIR)/cm7.elf: $(CM7)/src/main.c $(COMMON)/src/utils.c $(PCH_M7_DIR)/common.h.gch
	@echo "$(CYAN)  CC  [M7]$(NC)"
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(M7_FLAGS) -T $(CM7)/layout.ld $(CM7)/src/main.c $(COMMON)/src/utils.c -o $@

$(BUILD_DIR)/cm4.elf: $(CM4)/src/main.c $(COMMON)/src/utils.c $(PCH_M4_DIR)/common.h.gch
	@echo "$(CYAN)  CC  [M4]$(NC)"
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(M4_FLAGS) -T $(CM4)/layout.ld $(CM4)/src/main.c $(COMMON)/src/utils.c -o $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@$(OBJCOPY) -O binary $< $@

flash:
	@./scripts/flash.sh
wipe:
	@./scripts/wipe.sh
debug-server:
	@./scripts/debug_server.sh
debug:
	@./scripts/debug_client.sh $(CORE)

clean:
	@rm -rf $(BUILD_DIR)
