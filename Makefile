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
OPT_FLAGS := $(if $(filter fast,$(OPT)),-O3,$(if $(filter small,$(OPT)),-Os,-O0))

# --- Paths ---
BUILD_DIR  := build
OBJ_DIR    := $(BUILD_DIR)/obj
COMMON     := common
CM7        := cm7
CM4        := cm4
PCH_M7_DIR := $(BUILD_DIR)/pch/cm7
PCH_M4_DIR := $(BUILD_DIR)/pch/cm4

# --- Source Discovery ---
COMMON_SRCS := $(wildcard $(COMMON)/src/*.c)
M7_SRCS     := $(wildcard $(CM7)/src/*.c)
M4_SRCS     := $(wildcard $(CM4)/src/*.c)
COMMON_INC  := $(COMMON)/inc

# --- Object Mapping (The Secret Sauce) ---
# This ensures every .c file has a unique .o path so they don't overwrite each other
M7_OBJS        := $(patsubst $(CM7)/src/%.c, $(OBJ_DIR)/m7/%.o, $(M7_SRCS))
M4_OBJS        := $(patsubst $(CM4)/src/%.c, $(OBJ_DIR)/m4/%.o, $(M4_SRCS))
COMMON_OBJS_M7 := $(patsubst $(COMMON)/src/%.c, $(OBJ_DIR)/m7/common/%.o, $(COMMON_SRCS))
COMMON_OBJS_M4 := $(patsubst $(COMMON)/src/%.c, $(OBJ_DIR)/m4/common/%.o, $(COMMON_SRCS))

# --- Flags ---
CFLAGS_BASE := -mthumb -g $(OPT_FLAGS) -Wall -nostartfiles -Winvalid-pch \
               -I$(COMMON_INC) -I$(CM7)/inc -I$(CM4)/inc

M7_FLAGS := -mcpu=cortex-m7 -DCORE_CM7 $(CFLAGS_BASE) -I$(PCH_M7_DIR)
M4_FLAGS := -mcpu=cortex-m4 -DCORE_CM4 $(CFLAGS_BASE) -I$(PCH_M4_DIR)

# --- Primary Targets ---
.PHONY: all clean stats-zero stats-report build_binaries

all: stats-zero
	@$(MAKE) --no-print-directory build_binaries
	@echo "$(GREEN)Build finished successfully.$(NC)"
	@$(MAKE) --no-print-directory stats-report

build_binaries: $(BUILD_DIR)/cm7.bin $(BUILD_DIR)/cm4.bin
	@$(SIZE) -B $(BUILD_DIR)/*.elf

# Linking
$(BUILD_DIR)/cm7.elf: $(M7_OBJS) $(COMMON_OBJS_M7)
	@echo "$(CYAN)  LINK [M7]$(NC)"
	@mkdir -p $(dir $@)
	@$(CC) $(M7_FLAGS) -T $(CM7)/layout.ld $^ -o $@

$(BUILD_DIR)/cm4.elf: $(M4_OBJS) $(COMMON_OBJS_M4)
	@echo "$(CYAN)  LINK [M4]$(NC)"
	@mkdir -p $(dir $@)
	@$(CC) $(M4_FLAGS) -T $(CM4)/layout.ld $^ -o $@

# Compilation Rules (M7)
$(OBJ_DIR)/m7/%.o: $(CM7)/src/%.c $(PCH_M7_DIR)/common.h.gch
	@echo "  CC [M7] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M7_FLAGS) -c $< -o $@

$(OBJ_DIR)/m7/common/%.o: $(COMMON)/src/%.c $(PCH_M7_DIR)/common.h.gch
	@echo "  CC [M7-Common] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M7_FLAGS) -c $< -o $@

# Compilation Rules (M4)
$(OBJ_DIR)/m4/%.o: $(CM4)/src/%.c $(PCH_M4_DIR)/common.h.gch
	@echo "  CC [M4] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M4_FLAGS) -c $< -o $@

$(OBJ_DIR)/m4/common/%.o: $(COMMON)/src/%.c $(PCH_M4_DIR)/common.h.gch
	@echo "  CC [M4-Common] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M4_FLAGS) -c $< -o $@

# Precompiled Headers
$(PCH_M7_DIR)/common.h.gch: $(COMMON_INC)/common.h
	@mkdir -p $(PCH_M7_DIR)
	@$(CC) $(M7_FLAGS) -x c-header $< -o $@

$(PCH_M4_DIR)/common.h.gch: $(COMMON_INC)/common.h
	@mkdir -p $(PCH_M4_DIR)
	@$(CC) $(M4_FLAGS) -x c-header $< -o $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@$(OBJCOPY) -O binary $< $@

# --- Diagnostics ---
stats-zero:
	@if [ ! -z "$(CCACHE)" ]; then $(CCACHE) -z > /dev/null; fi

stats-report:
	@echo "$(YELLOW)--- Ccache Efficiency ---$(NC)"
	@if [ ! -z "$(CCACHE)" ]; then \
		$(CCACHE) -s ; \
	else \
		echo "ccache not enabled."; \
	fi
	@echo "$(YELLOW)-------------------------$(NC)"

clean:
	@rm -rf $(BUILD_DIR)

flash:
	@./scripts/flash.sh
wipe:
	@./scripts/wipe.sh
debug-server:
	@./scripts/debug_server.sh
debug:
	@./scripts/debug_client.sh $(CORE)

