KERNEL_DIR=/work/system/linux-2.6.22.6
ROOT_NFS=/work/root_qtopia-jz-mini2440
MAKE=make
CC=arm-linux-gcc

#改这里
APP=test15.c   			#测试程序
APP_TARGET=test15
OBJ=15_nor.o



all:
	$(MAKE) -C $(KERNEL_DIR) M=`pwd` modules
	$(CC) $(APP) -o $(APP_TARGET)
clean:
	$(MAKE) -C $(KERNEL_DIR) M=`pwd` modules clean
	rm -rf Module.symvers
	rm -rf $(APP_TARGET)
	rm -rf *.o *.ko *.mod.c
cl:
	$(MAKE) -C $(KERNEL_DIR) M=`pwd` modules clean
	rm -rf Module.symvers
	rm -rf $(APP_TARGET)
	rm -rf *.o *.ko
	rm -rf *.o *.ko *.mod.c
cp:
	cp *.ko $(ROOT_NFS)
	cp $(APP_TARGET) $(ROOT_NFS)

obj-m += $(OBJ)
