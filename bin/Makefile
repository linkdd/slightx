APPS_BIN = posix-test
KERNEL_BIN = kernel.bin
LIBC_BIN = libc.a
.PHONY: install clean

install:
	@if [ ! -d ../mnt/sys/ ] ; then mkdir ../mnt/sys/ ; fi
	@if [ ! -d ../mnt/bin/ ] ; then mkdir ../mnt/bin/ ; fi
	@cp -f $(KERNEL_BIN) $(LIBC_BIN) ../mnt/sys/
	@cp -f $(APPS_BIN) ../mnt/bin/

clean:
	@rm -rf kernel.bin libc.a
