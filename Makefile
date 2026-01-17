include build/mk/config.mk
include build/mk/common.mk


.PHONY: all
all:
	@echo "Targets:"
	@echo "    configure       Run CMake"
	@echo "    build           Build SlightX Operating System"
	@echo "    install         Build SlightX Operating System and generate ISO image"
	@echo "    clean           Remove build artifacts"
	@echo "    vm.start        Start QEMU virtual machine"
	@echo "    vm.startgdb     Start QEMU virtual machine with GDB session"
	@echo "    vm.attachgdb    Attach GDB session to QEMU virtual machine"


include build/mk/targets.mk
