KDIR := /home/teddy/work/yocto_r3/kernel/kernel-4.4.x
PWD := $(shell pwd)
 
obj-m += hello.o
 
all:
	make -C $(KDIR) M=$(PWD) modules
 
clean:
	make -C $(KDIR) M=$(PWD) clean
