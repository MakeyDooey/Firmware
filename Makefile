# ~/Firmware/Makefile
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -mcpu=cortex-m7 -mthumb -g -O0
LDFLAGS = -T cm7/layout.ld -nostartfiles

BUILD_DIR = build

all: $(BUILD_DIR)/blink.bin

$(BUILD_DIR)/blink.elf: cm7/main.c cm7/layout.ld
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) cm7/main.c -o $@

$(BUILD_DIR)/blink.bin: $(BUILD_DIR)/blink.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)

flash: $(BUILD_DIR)/blink.bin
	openocd -f openocd.cfg -c "program $(BUILD_DIR)/blink.bin verify reset exit 0x08000000"
