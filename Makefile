# Makefile for FreeX

export CC = gcc
export AS = nasm
export LD = ld

export CFLAGS = -fno-builtin -m32 -Wall -nostartfiles -nostdlib -nostdinc -g -c
export ASFLAGS = -f elf
export LDFLAGS = -melf_i386 -T linker.ld

.PHONY: all clean install run

all:
	@make all -C boot
	@make all -C kernel
	@make all -C libc
	@make all -C apps

clean:
	@make clean -C boot
	@make clean -C kernel
	@make clean -C libc
	@make clean -C apps
	@rm -rf bin/*

install:
	@losetup /dev/loop0 bin/floppy.img
	@mount /dev/loop0 mnt
	@cp bin/kernel.bin mnt/sys/
	@umount /dev/loop0
	@losetup -d /dev/loop0

run:
	@/sbin/losetup /dev/loop0 bin/floppy.img
	@bochs -f bochsrc.txt
	@/sbin/losetup -d /dev/loop0
