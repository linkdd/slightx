$(CMAKEDIR):
	@mkdir -p $@


.PHONY: configure
configure: $(CMAKEDIR)
	@(cd $(CMAKEDIR) && $(CMAKE) $(ROOTDIR) $(CMAKEFLAGS))


.PHONY: build
build: configure
	@$(MAKE) -C $(CMAKEDIR)


.PHONY: install
install: configure
	@$(MAKE) -C $(CMAKEDIR) install


.PHONY: clean
clean:
	@rm -vrf $(CMAKEDIR) $(SYSROOT) $(DISTDIR)


.PHONY: vm.start
vm.start:
	@$(VM) $(VMFLAGS)


.PHONY: vm.startgdb
vm.startgdb:
	@$(VM) $(VMFLAGS) -s -S


.PHONY: vm.attachgdb
vm.attachgdb:
	@gdb -x $(ROOTDIR)/toolchain/session.gdb
