# obj-m += irq.o
 
# KDIR = /lib/modules/$(shell uname -r)/build
 
# all:
# 	make -C $(KDIR)  M=$(shell pwd) modules
 
# clean:
# 	make -C $(KDIR)  M=$(shell pwd) clean

KDIR := /home/teddy/work/yocto_r3/kernel/kernel-4.4.x
PWD := $(shell pwd)
 
obj-m += irq.o
 
all:
	make -C $(KDIR) M=$(PWD) modules
 
clean:
	make -C $(KDIR) M=$(PWD) clean
