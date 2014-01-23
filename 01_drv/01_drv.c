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

#ifdef CONFIG_KMOD
#include <linux/kmod.h>
#endif


struct class *first_class;
struct class_device *first_class_dev;

int first_open (struct inode *inode, struct file *file)
{
	printk("This is first_open.\n");
	return 0;
}


ssize_t first_read (struct file *file, char __user *user, size_t size, loff_t *loff)
{
	printk("This is first_read.\n");
	return 0;
}


ssize_t first_write (struct file *file, const char __user *user, size_t size, loff_t *loff)
{
	printk("This is first_write.\n");
	return 0;
}

struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = first_open,
	.read = first_read,
	.write = first_write,

};

int major;
void first_init(void)
{
	major = register_chrdev(0, "first_drv",&f_ops);	//名字不重要 cat /proc/devices时显示252设备号和名字

}

void first_exit(void)
{
	
	unregister_chrdev(major, "first_drv");       //名字不重要，"first"也可以
}

module_init(first_init)
module_exit(first_exit)
MODULE_LICENSE("GPL");





