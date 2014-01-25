KERNEL_DIR=/work/system/linux-2.6.22.6
ROOT_NFS=/home/student/root_qtopia
MAKE=make
CC=arm-linux-gcc

APP=test01c.c   	#测试程序
APP_TARGET=test01c



all:
	$(MAKE) -C $(KERNEL_DIR) M=`pwd` modules
	$(CC) $(APP) -o $(APP_TARGET)
clean:
	$(MAKE) -C $(KERNEL_DIR) M=`pwd` modules clean
	rm -rf Module.symvers
	rm -rf $(APP_TARGET)
cl:
	$(MAKE) -C $(KERNEL_DIR) M=`pwd` modules clean
	rm -rf Module.symvers
	rm -rf $(APP_TARGET)
cp:
	cp *.ko $(ROOT_NFS)
	cp $(APP_TARGET) $(ROOT_NFS)

obj-m += 01c_drv.o
