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
//#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/kdev_t.h>
#include <linux/err.h>
#include <linux/slab.h>


#include <asm/io.h>		//ioremap

#include <asm/uaccess.h>	//copy_from_user

#include <linux/irq.h>		//request_irq

#include <linux/random.h>
#include <linux/interrupt.h>

#include <asm/irq.h>	//IRQT_BOTHEDGE
#include <linux/irq.h>


volatile unsigned long *gpgcon = NULL;
volatile unsigned long *gpgdat = NULL;


irqreturn_t key_handler(int irq, void *dev_id)
{
	printk("key pressed..%d:\n", irq);

	return IRQ_HANDLED;
}

int third_open (struct inode *inode, struct file *file)
{
	printk("third_open: \n");

	/* K1,K2,K3,K4--->EINT8,EINT11,EINT13,EINT14 */



	request_irq(IRQ_EINT8, key_handler, IRQT_BOTHEDGE, "K1", 1);
	request_irq(IRQ_EINT11, key_handler, IRQT_BOTHEDGE, "K2", 1);
	request_irq(IRQ_EINT13, key_handler, IRQT_BOTHEDGE, "K3", 1);
	request_irq(IRQ_EINT14, key_handler, IRQT_BOTHEDGE, "K4", 1);

	return 0;
}


ssize_t third_read (struct file *file, char __user *user, size_t size, loff_t *loff)
{
	//printk("This is third_read.\n");

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

ssize_t third_write (struct file *file, const char __user *user, size_t size, loff_t *loff)
{
	printk("This is third_write.\n");

	return 0;
}

int third_release (struct inode *inode, struct file *file)
{

	free_irq(IRQ_EINT8, 1);
	free_irq(IRQ_EINT11, 1);
	free_irq(IRQ_EINT13, 1);
	free_irq(IRQ_EINT14, 1);

	return 0;
}

struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = third_open,
	.read = third_read,
	.write = third_write,
	.release = third_release,

};

int major;
struct class 		*third_class;
struct class_device 	*third_class_device;

int third_init(void)
{
	major = register_chrdev(0, "third_drv",&f_ops);	//名字不重要 cat /proc/devices时显示252设备号和名字

	third_class = class_create(THIS_MODULE, "third_key_drv");

	third_class_device = class_device_create(third_class, NULL, MKDEV(major,0), NULL, "keys");

	gpgcon = (volatile unsigned long *)
				ioremap(0x56000060, sizeof(unsigned long));		//映射虚拟地址

	gpgdat = (volatile unsigned long *)
				ioremap(0x56000064, sizeof(unsigned long));


	return 0;
}

void third_exit(void)
{
	unregister_chrdev(major, "third_drv");       //名字不重要，"sec"也可以

	class_device_unregister(third_class_device);	//先unregister, 再destroy
	
	class_destroy(third_class);

	iounmap(gpgcon);
	iounmap(gpgdat);
}

module_init(third_init)
module_exit(third_exit)
MODULE_LICENSE("GPL");

