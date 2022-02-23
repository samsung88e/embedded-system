obj-m += test_sort.o

KDIR := /lib/modules/$(shell uname -r)/build

test_sort:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean