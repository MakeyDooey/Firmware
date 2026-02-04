# --- Docker Configuration ---
DOCKER_IMAGE := stm32-h7-builder
# Persistent cache directory on your local Linux machine
HOST_CCACHE  := $(HOME)/.ccache_stm32

# DOCKER_RUN Explained:
# -u: Runs as YOU (prevents 'root' owned files in build/)
# -v $(CURDIR): Mounts project code
# -v $(HOST_CCACHE): Mounts the 'brain' (cache) so it survives container restarts
DOCKER_RUN   := docker run --rm \
                -u $(shell id -u):$(shell id -g) \
                -v $(CURDIR):/workspace \
                -v $(HOST_CCACHE):/workspace/.ccache \
                $(DOCKER_IMAGE)

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
FREERTOS   := middlewares/freertos
HAL        := middlewares/hal
PCH_M7_DIR := $(BUILD_DIR)/pch/cm7
PCH_M4_DIR := $(BUILD_DIR)/pch/cm4

# --- Source Discovery ---
COMMON_SRCS := $(wildcard $(COMMON)/src/*.c)
M7_C_SRCS   := $(wildcard $(CM7)/src/*.c)
M7_S_SRCS   := $(wildcard $(CM7)/src/*.s)
M4_C_SRCS   := $(wildcard $(CM4)/src/*.c)
M4_S_SRCS   := $(wildcard $(CM4)/src/*.s)
COMMON_INC  := $(COMMON)/inc

# --- FreeRTOS (CM7 only) ---
FREERTOS_SRCS := \
        $(FREERTOS)/list.c \
        $(FREERTOS)/queue.c \
        $(FREERTOS)/tasks.c \
        $(FREERTOS)/timers.c \
        $(FREERTOS)/portable/GCC/ARM_CM7/r0p1/port.c \
        $(FREERTOS)/portable/MemMang/heap_4.c \
        $(FREERTOS)/cli/FreeRTOS_CLI.c

# --- Object Mapping ---
M7_OBJS        := $(patsubst $(CM7)/src/%.c,$(OBJ_DIR)/m7/%.o,$(M7_C_SRCS))
M7_OBJS        += $(patsubst $(CM7)/src/%.s,$(OBJ_DIR)/m7/%.o,$(M7_S_SRCS))

M4_OBJS        := $(patsubst $(CM4)/src/%.c,$(OBJ_DIR)/m4/%.o,$(M4_C_SRCS))
M4_OBJS        += $(patsubst $(CM4)/src/%.s,$(OBJ_DIR)/m4/%.o,$(M4_S_SRCS))

COMMON_OBJS_M7 := $(patsubst $(COMMON)/src/%.c,$(OBJ_DIR)/m7/common/%.o,$(COMMON_SRCS))
COMMON_OBJS_M4 := $(patsubst $(COMMON)/src/%.c,$(OBJ_DIR)/m4/common/%.o,$(COMMON_SRCS))

FREERTOS_OBJS  := $(patsubst %.c,$(OBJ_DIR)/m7/%.o,$(FREERTOS_SRCS))

# --- Flags ---
CFLAGS_BASE := -mthumb -g $(OPT_FLAGS) -Wall -nostartfiles -Winvalid-pch \
        -I$(COMMON_INC) \
        -I$(CM7)/inc -I$(CM4)/inc \
        -I$(HAL)/inc \
        -I$(FREERTOS)/include \
        -I$(FREERTOS)/portable/GCC/ARM_CM7/r0p1 \
        -I$(FREERTOS)/cli

M7_FLAGS := -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -DCORE_CM7 $(CFLAGS_BASE) -I$(PCH_M7_DIR)
M7_ASFLAGS := -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard
M4_FLAGS := -mcpu=cortex-m4 -DCORE_CM4 $(CFLAGS_BASE) -I$(PCH_M4_DIR)

# --- Helper Script Mappings ---
FLASH_SCRIPT := scripts/makefile/flash.sh
WIPE_SCRIPT  := scripts/makefile/wipe.sh
DB_SERVER    := scripts/makefile/debug_server.sh
DB_CLIENT    := scripts/makefile/debug_client.sh

# --- Primary Targets ---
.PHONY: all clean stats-zero stats-report build_binaries docker-image docker-build flash wipe debug

all: stats-zero build_binaries stats-report

# Professional Docker Targets
docker-image:
	docker build -t $(DOCKER_IMAGE) .

docker-build:
	@mkdir -p $(HOST_CCACHE)
	$(DOCKER_RUN) make all OPT=$(OPT)

build_binaries: $(BUILD_DIR)/cm7.bin $(BUILD_DIR)/cm4.bin
	@$(SIZE) -B $(BUILD_DIR)/*.elf

# --- Linking ---
$(BUILD_DIR)/cm7.elf: $(M7_OBJS) $(COMMON_OBJS_M7) $(FREERTOS_OBJS)
	@echo "$(CYAN)  LINK [M7]$(NC)"
	@mkdir -p $(dir $@)
	@$(CC) $(M7_FLAGS) -T $(CM7)/layout.ld $^ -o $@

$(BUILD_DIR)/cm4.elf: $(M4_OBJS) $(COMMON_OBJS_M4)
	@echo "$(CYAN)  LINK [M4]$(NC)"
	@mkdir -p $(dir $@)
	@$(CC) $(M4_FLAGS) -T $(CM4)/layout.ld $^ -o $@

# --- Compilation Rules (M7) ---
$(OBJ_DIR)/m7/%.o: $(CM7)/src/%.c $(PCH_M7_DIR)/common.h.gch
	@echo "  CC [M7] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M7_FLAGS) -c $< -o $@

$(OBJ_DIR)/m7/%.o: $(CM7)/src/%.s
	@echo "  AS [M7] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M7_ASFLAGS) -x assembler-with-cpp -c $< -o $@

$(OBJ_DIR)/m7/common/%.o: $(COMMON)/src/%.c $(PCH_M7_DIR)/common.h.gch
	@echo "  CC [M7-Common] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M7_FLAGS) -c $< -o $@

$(OBJ_DIR)/m7/%.o: %.c
	@echo "  CC [M7-FR] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M7_FLAGS) -c $< -o $@

# --- Compilation Rules (M4) ---
$(OBJ_DIR)/m4/%.o: $(CM4)/src/%.c $(PCH_M4_DIR)/common.h.gch
	@echo "  CC [M4] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M4_FLAGS) -c $< -o $@

$(OBJ_DIR)/m4/%.o: $(CM4)/src/%.s
	@echo "  AS [M4] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M4_FLAGS) -x assembler-with-cpp -c $< -o $@

$(OBJ_DIR)/m4/common/%.o: $(COMMON)/src/%.c $(PCH_M4_DIR)/common.h.gch
	@echo "  CC [M4-Common] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(M4_FLAGS) -c $< -o $@

# --- Precompiled Headers ---
$(PCH_M7_DIR)/common.h.gch: $(COMMON_INC)/common.h
	@mkdir -p $(PCH_M7_DIR)
	@$(CC) $(M7_FLAGS) -x c-header $< -o $@

$(PCH_M4_DIR)/common.h.gch: $(COMMON_INC)/common.h
	@mkdir -p $(PCH_M4_DIR)
	@$(CC) $(M4_FLAGS) -x c-header $< -o $@

# --- Binary Output ---
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@$(OBJCOPY) -O binary $< $@

# --- Diagnostics ---
stats-zero:
	@if [ ! -z "$(CCACHE)" ]; then $(CCACHE) -z > /dev/null; fi

stats-report:
	@echo "$(YELLOW)--- Ccache Efficiency ---$(NC)"
	@if [ ! -z "$(CCACHE)" ]; then $(CCACHE) -s ; fi
	@echo "$(YELLOW)-------------------------$(NC)"
	
clean:
	@rm -rf $(BUILD_DIR)

# --- Hardware Targets (Run locally on Host) ---
flash:
	@chmod +x $(FLASH_SCRIPT)
	@./$(FLASH_SCRIPT)

wipe:
	@chmod +x $(WIPE_SCRIPT)
	@./$(WIPE_SCRIPT)

debug:
	@echo "Starting OpenOCD in separate process..."
	@chmod +x $(DB_SERVER)
	@./$(DB_SERVER) &
	@sleep 3
	@chmod +x $(DB_CLIENT)
	@./$(DB_CLIENT)
