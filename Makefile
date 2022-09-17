obj-m += gmh.o
CFLAGS_gmh.o := -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

ins:
	sudo insmod gmh.ko
	sudo chmod 666 /dev/gmh

rem:
	sudo rmmod gmh
