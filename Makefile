SOURCES = $(wildcard src/*.c) $(wildcard src/*/*.c)
ASOURCES = $(wildcard asm/*.S)
OBJECTS = $(SOURCES:%.c=%.o)
AOBJECTS = $(ASOURCES:%.S=%.o)
BUILD_DIR = build
BUILDOBJECTS := $(patsubst %,$(BUILD_DIR)/%,$(SOURCES:.c=.o))
BUILDAOBJECTS := $(patsubst %,$(BUILD_DIR)/%,$(ASOURCES:.c=.o))
CFLAGS = -nostdlib -nostartfiles -ffreestanding -mgeneral-regs-only

all: build

run: build
	qemu-system-aarch64 -m 1024 -M raspi3 -serial stdio -kernel kernel8.img

%.o: %.c
	mkdir -p $(BUILD_DIR)
	../gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc $(CFLAGS) -c $< -o $(BUILD_DIR)/$(notdir $@) -O2 -Wall -Wextra

%.o: %.S
	../gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc $(CFLAGS) -c $< -o $(BUILD_DIR)/$(notdir $@)

BUILD_OBJECTS = $(wildcard $(BUILD_DIR)/*.o)

build: $(OBJECTS) $(AOBJECTS)
	mkdir -p $(BUILD_DIR)/asm
	../gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc -T other/linker.ld -o build/aarch64_firmware.elf -ffreestanding -O2 -nostdlib $(BUILD_OBJECTS)
	../gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin/aarch64-none-elf-objcopy -O binary build/aarch64_firmware.elf kernel8.img
clean:
	rm -r build
