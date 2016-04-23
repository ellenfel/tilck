
export AS = nasm
export CC = gcc
export OPT = -O2 -fvisibility=default -Wall -Wextra
export INCDIRS = -I$(shell pwd)/include
export CFLAGS =  $(OPT) -std=c99 $(INCDIRS) -m32 -mno-red-zone -ffreestanding -g \
                 -nostdinc -fno-builtin  -fno-asynchronous-unwind-tables \
			        -fno-zero-initialized-in-bss -Wno-unused-function

export DEPDIR := .d
export DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

export BUILD_DIR = $(shell pwd)/build

export COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) -c
export POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

BOOTLOADER_TARGET = $(BUILD_DIR)/bootloader.bin
KERNEL_UNSTRIPPED = $(BUILD_DIR)/uk
KERNEL_TARGET = $(BUILD_DIR)/kernel32.bin
FINAL_TARGET = os2.img

all: $(BUILD_DIR) $(BOOTLOADER_TARGET) $(KERNEL_TARGET)
	dd status=noxfer conv=notrunc if=$(BOOTLOADER_TARGET) of=$(FINAL_TARGET)
	dd status=noxfer conv=notrunc if=$(KERNEL_TARGET) of=$(FINAL_TARGET) seek=8 obs=512 ibs=512

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	dd status=noxfer if=/dev/zero of=$(FINAL_TARGET) obs=512 ibs=512 count=2880
	
clean:
	rm -rf $(BUILD_DIR) $(FINAL_TARGET)

# Targets that do not generate files
.PHONY: all clean $(KERNEL_TARGET)

$(KERNEL_TARGET):
	cd src && $(MAKE)

$(BOOTLOADER_TARGET): bootloader/boot_stage1.asm bootloader/boot_stage2.asm
	nasm -f bin -o $(BUILD_DIR)/boot_stage1.bin bootloader/boot_stage1.asm
	nasm -f bin -o $(BUILD_DIR)/boot_stage2.bin bootloader/boot_stage2.asm
	cp $(BUILD_DIR)/boot_stage1.bin $(BOOTLOADER_TARGET)
	dd status=noxfer conv=notrunc if=$(BUILD_DIR)/boot_stage2.bin of=$(BOOTLOADER_TARGET) seek=1 obs=512 ibs=512
