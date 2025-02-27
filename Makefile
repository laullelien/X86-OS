.PHONY: clean all

### QEMU
QEMU = /usr/bin/qemu-system-i386

### QEMU opt
QEMUOPTSRUN = -machine q35 -m 256 -kernel kernel/kernel.bin

# Output directory for each submakefiles
OUTPUT := out
export OUTPUT

#
# Some build tools need to be explicitely defined before building. The toolchain
# creates the following platform tools configuration file before it allows the
# toolchain to build.
#
PLATFORM_TOOLS := $(OUTPUT)/platform-tools.mk
export PLATFORM_TOOLS

all: | kernel/$(PLATFORM_TOOLS) user/$(PLATFORM_TOOLS)
	$(MAKE) -C user/ all VERBOSE=$(VERBOSE)
	$(MAKE) -C kernel/ kernel.bin VERBOSE=$(VERBOSE)

kernel/$(PLATFORM_TOOLS):
	$(MAKE) -C kernel/ $(PLATFORM_TOOLS)

user/$(PLATFORM_TOOLS):
	$(MAKE) -C user/ $(PLATFORM_TOOLS)

cleanUser:
	$(MAKE) clean -C user/

cleanKernel:
	$(MAKE) clean -C kernel/
	
clean: cleanKernel cleanUser

user: cleanUser all

run: all
	$(QEMU) $(QEMUOPTSRUN)

debug: all
	qemu-system-i386 -machine q35 -debugcon stdio -s -S -m 256M -kernel kernel/kernel.bin & 
	
	exec gdb kernel/kernel.bin -ex 'target remote :1234'
