#include <linux/fs.h>
#include <linux/module.h>

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

#ifdef CONFIG_KMOD
#include <linux/kmod.h>
#endif


#include <linux/device.h>   //class_create()
#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/kdev_t.h>
#include <linux/err.h>
#include <linux/slab.h>


#include <asm/io.h>		//ioremap

#include <asm/uaccess.h>	//copy_from_user

volatile unsigned long *GPBCON = NULL;
volatile unsigned long *GPBDAT = NULL;

#define DBG printk
//#define DBG(KERN_DEBUGx...)
int first_open (struct inode *inode, struct file *file)
{
	printk("first_open: GPBCON output\n");
	DBG(KERN_DEBUG"%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
	/* GPB5,6,7,8 分别接LED01，LED02，LED03，LED04，配置成输出*/
	*GPBCON &= ~(0x1<<11 );		//GPB5配置成输出[11:10] = 01
	*GPBCON |= (0x1<<10 );		//GPB5配置成输出[11:10] = 01

	*GPBCON &= ~(0x1<<13 );		//GPB6配置成输出[13:12] = 01
	*GPBCON |= (0x1<<12 );		//GPB6配置成输出[13:12] = 01

	*GPBCON &= ~(0x1<<15 );		//GPB7配置成输出[15:14] = 01
	*GPBCON |= (0x1<<14 );		//GPB7配置成输出[15:14] = 01

	DBG(KERN_DEBUG"%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
	return 0;
}


ssize_t first_read (struct file *file, char __user *user, size_t size, loff_t *loff)
{
	printk("This is first_read.\n");
	return 0;
}

int val;
ssize_t first_write (struct file *file, const char __user *user, size_t size, loff_t *loff)
{
	printk("This is first_write.\n");


	DBG(KERN_DEBUG"%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
	copy_from_user(&val, user, 4);
	if(val == 1)
	{
		//点灯
		printk("led on.val:%d\n", val);
	DBG(KERN_DEBUG"%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
		*GPBDAT &= ~(0x1<<5);				//亮LED1-->GPBDAT[5]=0
		*GPBDAT &= ~(0x1<<6);				//亮LED2-->GPBDAT[6]=0
		*GPBDAT &= ~(0x1<<7);				//亮LED3-->GPBDAT[7]=0
	}
	else if(val == 0)
	{
		//灭灯
		printk("led off.val:%d\n", val);
	DBG(KERN_DEBUG"%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
		*GPBDAT |= (0x1<<5); 				//灭LED1
		*GPBDAT |= (0x1<<6); 				//灭LED2
		*GPBDAT |= (0x1<<7); 				//灭LED3
	}


	return 0;
}

struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = first_open,
	.read = first_read,
	.write = first_write,

};

int major;
struct class 		*first_class;
struct class_device 	*first_class_device;

int first_init(void)
{
	DBG(KERN_DEBUG"%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
	major = register_chrdev(0, "first_drv",&f_ops);	//名字不重要 cat /proc/devices时显示252设备号和名字

	first_class = class_create(THIS_MODULE, "firstdrv");

	first_class_device = class_device_create(first_class, NULL, MKDEV(major,0), NULL, "xyz");

	DBG(KERN_DEBUG"%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
	GPBCON = (volatile unsigned long *)0x56000010;
				//ioremap(0x56000010, sizeof(unsigned long));		//映射虚拟地址

	GPBDAT = (volatile unsigned long *)
				ioremap(0x56000014, sizeof(unsigned long));


	return 0;
}

void first_exit(void)
{
	unregister_chrdev(major, "first_drv");       //名字不重要，"first"也可以

	class_device_unregister(first_class_device);	//先unregister, 再destroy
	
	class_destroy(first_class);

	iounmap(GPBCON);
	iounmap(GPBDAT);
}

module_init(first_init)
module_exit(first_exit)
MODULE_LICENSE("GPL");

