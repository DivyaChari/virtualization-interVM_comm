obj-m := bridge_comm.o

#KERNEL_DIR=$(PWD)/linux/


default:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
#make ARCH=x86 -C ${KERNEL_DIR} M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
