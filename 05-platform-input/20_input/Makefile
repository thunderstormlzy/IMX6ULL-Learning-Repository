KERNELDIR := /home/lzy/linux/uboot/linux_nxp
CURRENT_PATH := $(shell pwd)
obj-m := input.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
