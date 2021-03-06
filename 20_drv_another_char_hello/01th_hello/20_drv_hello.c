#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/smp_lock.h>
#include <linux/seq_file.h>
#include <linux/kobject.h>
#include <linux/kobj_map.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/backing-dev.h>
#include <linux/device.h>

//1.确定主设备号
static int major;


int hello_open (struct inode *inode, struct file *file)
{
	printk("hello_open\n");
	return 0;
}

//2.构造file_operations结构体
static struct file_operations hello_fops= {
	.owner = THIS_MODULE,
	.open = hello_open,
	
};

#define HELLO_CNT 2
static struct cdev hello_cdev;
static struct class *cls;

static int hello_init(void)
{
//3.告诉内核
	dev_t devid;
#if 0
	register_chrdev(0, "hello", &hello_fops);
#else
	if (major) {
		devid = MKDEV(major, 0);
		register_chrdev_region(devid, HELLO_CNT, "hello");
		//(major, 0~1)对应hello_fops，其他的(major,1~255)都不对应
	} else {
		alloc_chrdev_region(&devid, 0, HELLO_CNT, "hello");
		major = MAJOR(devid);
	}
	cdev_init(&hello_cdev, &hello_fops);
	cdev_add(&hello_cdev, devid, HELLO_CNT);
#endif

	cls = class_create(THIS_MODULE, "hello");
	class_device_create(cls, NULL, MKDEV(major,0), NULL, "hello0");		//   /dev/hello0
	class_device_create(cls, NULL, MKDEV(major,1), NULL, "hello1");		//   /dev/hello1
	class_device_create(cls, NULL, MKDEV(major,2), NULL, "hello2");		//   /dev/hello2


	return 0;
}

static void hello_exit(void)
{

	class_device_destroy(cls,  MKDEV(major,0));		
	class_device_destroy(cls,  MKDEV(major,1));		
	class_device_destroy(cls,  MKDEV(major,2));		
	class_destroy(cls);

	cdev_del(&hello_cdev);
	unregister_chrdev_region(MKDEV(major, 0), HELLO_CNT);
}

module_init(hello_init)
module_exit(hello_exit)
MODULE_LICENSE("GPL");
