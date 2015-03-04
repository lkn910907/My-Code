obj-m := dom-grant.o
kernel_path=/usr/src/linux-3.2
all:
#this make calls this Makefile, and the command make calls the Makefile in the 
#kernel_path to make modules
	make -C $(kernel_path) M=$(PWD) modules
clean:
#this make calls this Makefile, and the command make calls the Makefile in the 
#kernel_path to make clean
	make -C $(kernel_path) M=$(PWD) clean
