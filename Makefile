TARGET = kernel.bin
ISO = ArchaOS.iso

CC = gcc
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -nostdlib -mno-red-zone
LD = ld
LDFLAGS = -T src/linker.ld -nostdlib -melf_i386
ASM = nasm

SRC = src/kernel.c src/vga.c
OBJ = build/kernel.o build/vga.o build/boot.o

all: $(ISO)

build/kernel.o: src/kernel.c src/vga.h
	$(CC) $(CFLAGS) -c $< -o $@

build/vga.o: src/vga.c src/vga.h
	$(CC) $(CFLAGS) -c $< -o $@

build/boot.o: src/boot.asm
	$(ASM) -f elf32 $< -o $@

build/$(TARGET): $(OBJ) src/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

$(ISO): build/$(TARGET) iso/boot/grub/grub.cfg
	mkdir -p iso/boot
	cp build/$(TARGET) iso/boot/kernel.bin
	grub-mkrescue -o $(ISO) iso

clean:
	rm -rf build/*.o build/$(TARGET) $(ISO)
