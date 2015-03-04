obj-m := backend.o
#you can  modify the kernal_path for your own workload
kernel_path = /usr/src/linux-3.2
all:
	make -C $(kernel_path) M=$(PWD) modules
clean:
	make -C $(kernel_path) M=$(PWD) clean
