MAKEFLAGS += --no-print-directory

ROOTDIR = $(PWD)
CMAKEDIR = $(ROOTDIR)/__build__/$(BUILD_TYPE)
SYSROOT = $(ROOTDIR)/sysroot/$(BUILD_TYPE)
DISTDIR = $(ROOTDIR)/dist/$(BUILD_TYPE)

VM = qemu-system-x86_64
VMFLAGS = -cdrom $(DISTDIR)/slightx.iso -m 512M -smp cpus=4

CMAKE = cmake
CMAKEFLAGS =                        \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)  \
	-DCMAKE_C_COMPILER=$(CC)
