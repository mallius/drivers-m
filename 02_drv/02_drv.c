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


volatile unsigned long *gpgcon = NULL;
volatile unsigned long *gpgdat = NULL;


int sec_open (struct inode *inode, struct file *file)
{
	printk("sec_open: gpgcon input\n");
	/* gpgcon0,3,5,6为输入引脚 */

	*gpgcon &= ~((0x3<< 0) | (0x3<< 6) | (0x3<<10) | (0x3<<12));

	return 0;
}


ssize_t sec_read (struct file *file, char __user *user, size_t size, loff_t *loff)
{
	//printk("This is sec_read.\n");

	unsigned char key_val[4];
	int reg_val;

	reg_val = *gpgdat;
	
	if(size != sizeof(key_val))
		return -EINVAL;

	key_val[0] = (reg_val & (1<< 0)) ? 1: 0;
	key_val[1] = (reg_val &	(1<< 3)) ? 1: 0;
	key_val[2] = (reg_val & (1<< 5)) ? 1: 0;
	key_val[3] = (reg_val &	(1<< 6)) ? 1: 0;


	copy_to_user(user, key_val, sizeof(key_val));

	return sizeof(key_val);

}

ssize_t sec_write (struct file *file, const char __user *user, size_t size, loff_t *loff)
{
	printk("This is sec_write.\n");

	return 0;
}

struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = sec_open,
	.read = sec_read,
	.write = sec_write,

};

int major;
struct class 		*sec_class;
struct class_device 	*sec_class_device;

int sec_init(void)
{
	major = register_chrdev(0, "sec_drv",&f_ops);	//名字不重要 cat /proc/devices时显示252设备号和名字

	sec_class = class_create(THIS_MODULE, "secdrv");

	sec_class_device = class_device_create(sec_class, NULL, MKDEV(major,0), NULL, "keys");

	gpgcon = (volatile unsigned long *)
				ioremap(0x56000060, sizeof(unsigned long));		//映射虚拟地址

	gpgdat = (volatile unsigned long *)
				ioremap(0x56000064, sizeof(unsigned long));


	return 0;
}

void sec_exit(void)
{
	unregister_chrdev(major, "sec_drv");       //名字不重要，"sec"也可以

	class_device_unregister(sec_class_device);	//先unregister, 再destroy
	
	class_destroy(sec_class);

	iounmap(gpgcon);
	iounmap(gpgdat);
}

module_init(sec_init)
module_exit(sec_exit)
MODULE_LICENSE("GPL");

