BIN = bin
KERNEL = bamboo-os
IMAGE = image.hdd

IMGFILES = $(shell find img -mindepth 1 -maxdepth 1 -type f,d)

CC = gcc
CFLAGS = -Wall -Wextra -O0 -mcmodel=kernel -mno-red-zone -ffreestanding -nostdlib -fno-PIC

ASM = nasm
ASMFLAGS = -Wall -F dwarf -f elf64

LD = ld
LDFLAGS = 
LDSCRIPT = linker.ld

SRC = src
OBJ = obj

SRCS = $(shell find src -name '*.c')
OBJS = $(patsubst $(SRC)/%, $(OBJ)/%, $(patsubst %.c, %.o, $(SRCS)))

ASMSRCS = $(shell find src -name '*.asm')
ASMOBJS = $(patsubst $(SRC)/%, $(OBJ)/%, $(patsubst %.asm, %_asm.o, $(ASMSRCS)))

LIMINE = limine

.PHONY: all, limine

all: $(BIN)/$(IMAGE)

$(BIN)/$(IMAGE): limine $(BIN)/$(KERNEL)
	@rm -f $(BIN)/$(IMAGE)
	@dd if=/dev/zero bs=1M count=64 of=$(BIN)/$(IMAGE)
	
	@sgdisk $(BIN)/$(IMAGE) -n 1:0:+4M -t 1:ef00

	@./limine/limine bios-install $(BIN)/$(IMAGE)

	@mformat -i $(BIN)/$(IMAGE)@@1M

	@mmd -i $(BIN)/$(IMAGE)@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine
	@mcopy -i $(BIN)/$(IMAGE)@@1M $(BIN)/$(KERNEL) ::/boot

	@$(foreach isofile, $(IMGFILES), mcopy -i $(BIN)/$(IMAGE)@@1M -s $(isofile) ::;)
	@mcopy -i $(BIN)/$(IMAGE)@@1M limine/limine-bios.sys ::/boot/limine
	@mcopy -i $(BIN)/$(IMAGE)@@1M limine/BOOTX64.EFI ::/EFI/BOOT

$(BIN)/$(KERNEL): $(OBJS) $(ASMOBJS)
	@echo Linking kernel...
	@mkdir -p $(BIN)
	@$(LD) $(OBJS) $(ASMOBJS) $(LDFLAGS) -T $(LDSCRIPT) -o $@

$(OBJ)/%.o: $(SRC)/%.c
	@echo Compiling $<
	@mkdir -p $$(dirname $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/int/isr.o: $(SRC)/int/isr.c
	@echo Compiling $<
	@mkdir -p $$(dirname $@)
	@$(CC) $(CFLAGS) -mgeneral-regs-only -c $< -o $@

$(OBJ)/%_asm.o: $(SRC)/%.asm
	@echo Assembling $<
	@mkdir -p $$(dirname $@)
	@$(ASM) $(ASMFLAGS) $< -o $@

limine:
	@echo Downloading limine...
	-git clone https://github.com/limine-bootloader/limine --branch=v9.x-binary --depth=1 $(LIMINE) > /dev/null 2>&1
	@- make --directory=$(LIMINE)
	@echo 

run:
	sudo qemu-system-x86_64 -drive format=raw,file=$(BIN)/$(IMAGE) -machine q35 --smp 4

clean:
	rm -rf $(OBJ)
	rm -rf $(BIN)
	rm -f $(IMAGE)

deepclean: clean
	rm -rf limine/
