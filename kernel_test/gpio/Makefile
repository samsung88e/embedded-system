# obj-m += gpio2.o
 
# KDIR = /lib/modules/$(shell uname -r)/build
 
 
# all:
# 	make -C $(KDIR)  M=$(shell pwd) modules
 
# clean:
# 	make -C $(KDIR)  M=$(shell pwd) clean


KDIR := /home/teddy/work/yocto_r3/kernel/kernel-4.4.x
PWD := $(shell pwd)
 
obj-m += gpio.o
 
all:
	make -C $(KDIR) M=$(PWD) modules
 
clean:
	make -C $(KDIR) M=$(PWD) clean
