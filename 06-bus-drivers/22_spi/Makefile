KERNELDIR := /home/lzy/linux/uboot/linux_nxp
CURRENT_PATH := $(shell pwd)
obj-m := icm20608.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
