CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -mthumb -g -O0 -Icommon -nostartfiles

# Common objects
COMMON_SRC = common/utils.c

all: build/cm7.bin build/cm4.bin

build/cm7.elf: cm7/main.c $(COMMON_SRC) cm7/layout.ld
	mkdir -p build
	$(CC) $(CFLAGS) -mcpu=cortex-m7 -T cm7/layout.ld cm7/main.c $(COMMON_SRC) -o $@

build/cm4.elf: cm4/main.c $(COMMON_SRC) cm4/layout.ld
	mkdir -p build
	$(CC) $(CFLAGS) -mcpu=cortex-m4 -T cm4/layout.ld cm4/main.c $(COMMON_SRC) -o $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

flash: all
	openocd -f openocd.cfg \
	-c "program build/cm7.bin 0x08000000 verify" \
	-c "program build/cm4.bin 0x08100000 verify reset exit"

clean:
	rm -rf build
