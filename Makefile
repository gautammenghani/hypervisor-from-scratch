obj-m += gmh.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

ins:
	sudo insmod gmh.ko
	#sudo mknod /dev/gmh c 117 0
	#sudo chmod 666 /dev/gmh

rem:
	sudo rmmod gmh
	#sudo rm /dev/gmh
