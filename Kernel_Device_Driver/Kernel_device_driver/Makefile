KDIR := /home/teddy/work/yocto_r3/kernel/kernel-4.4.x
PWD := $(shell pwd)
 
obj-m += gpio_misc_driver.o
 
all:
	make -C $(KDIR) M=$(PWD) modules
 
clean:
	make -C $(KDIR) M=$(PWD) clean
