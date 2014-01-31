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

#include <asm/irq.h>		 //IRQT_BOTHEDGE
#include <linux/irq.h>

#include <asm/arch/gpio.h> 	 //s3c2410_gpio_getpin
#include <asm/arch/regs-gpio.h>  //S3C2410_GPG0,GPG3,GPG5,GPG6

#include <linux/wait.h>		//DECLARE_WAIT_QUEUE_HEAD
				//wait_event_interruptible
				//wake_up_interruptible

#include <linux/poll.h>		//poll-->poll_wait  , POLLIN
//#include <asm-generic/poll.h>

#include <asm/signal.h>		//SIGIO

#include <asm/atomic.h>		//ATOMIC_INIT(1)

#include <asm-generic/errno-base.h>

volatile unsigned long *gpgcon = NULL;
volatile unsigned long *gpgdat = NULL;

struct pin_des{
	unsigned int pin;
	unsigned int pin_val;
};

struct pin_des pins[4] = {
	{S3C2410_GPG0, 0X1},
	{S3C2410_GPG3, 0X2},
	{S3C2410_GPG5, 0X3},
	{S3C2410_GPG6, 0X4},
};

unsigned char keyval;
volatile int ev_press;
static DECLARE_WAIT_QUEUE_HEAD(key_waitirq);


struct fasync_struct *key_fasync;		//异步通知

atomic_t canopen = ATOMIC_INIT(1);

irqreturn_t key_handler(int irq, void *dev_id)
{
	unsigned int pinval;

	/*
	 *读出引脚值，是按下或是松开,按下引脚是低电平，松开引脚是高电平
	 */
//	printk("key pressed..irq:%d\n", irq);
	struct pin_des *ptr = (struct pin_des *)dev_id;

	pinval = s3c2410_gpio_getpin(ptr->pin);
	if(pinval)
	{
		/*松开,高电平*/
		keyval = 0x80 | (ptr->pin_val);
	}
	else
	{
		/*按下,低电平*/
		keyval = ptr->pin_val;
	}

	ev_press = 1;
	wake_up_interruptible(&key_waitirq);  //唤醒去read--->wait_up_interruptible--->copy_to_user


	kill_fasync(&key_fasync, SIGIO, POLLIN);


	return IRQ_HANDLED;
}

int sixth_open (struct inode *inode, struct file *file)
{
	printk("sixth_open: \n");

	/* K1,K2,K3,K4--->EINT8,EINT11,EINT13,EINT14 */

	if(!atomic_dec_and_test(&canopen))
	{
		atomic_inc(&canopen);		//回复原值会返回
		return -EBUSY;
	}

	request_irq(IRQ_EINT8, key_handler, IRQT_BOTHEDGE, "K1", &pins[0]);
	request_irq(IRQ_EINT11, key_handler, IRQT_BOTHEDGE, "K2", &pins[1]);
	request_irq(IRQ_EINT13, key_handler, IRQT_BOTHEDGE, "K3", &pins[2]);
	request_irq(IRQ_EINT14, key_handler, IRQT_BOTHEDGE, "K4", &pins[3]);

	return 0;
}


ssize_t sixth_read (struct file *file, char __user *user, size_t size, loff_t *loff)
{
	printk("sixth_read:\n");
	
	if(size != 1)
	{
		printk("size != 1\n");
		return -EINVAL;
	}

	wait_event_interruptible(key_waitirq, ev_press); //if ev_press == 0, 休眠，ev_press ==1,不休眠
							 //继续往下

	copy_to_user(user, &keyval, 1);
	ev_press = 0;

	return 1;

}

ssize_t sixth_write (struct file *file, const char __user *user, size_t size, loff_t *loff)
{
	printk("This is sixth_write.\n");

	return 0;
}

int sixth_release (struct inode *inode, struct file *file)
{

	atomic_inc(&canopen);

	free_irq(IRQ_EINT8, &pins[0]);
	free_irq(IRQ_EINT11, &pins[1]);
	free_irq(IRQ_EINT13, &pins[2]);
	free_irq(IRQ_EINT14, &pins[3]);

	return 0;
}

static unsigned int sixth_poll(struct file *file, poll_table *wait)
{

	unsigned int mask = 0;

	poll_wait(file, &key_waitirq, wait);

	
	if(ev_press)			//ev_press == 1 --> 有按键按下
	{
		mask |= POLLIN | POLLRDNORM ;
	}

	return mask;
}



int sixth_fasync(int fd, struct file *file, int on)
{
	printk("sixth_fasync:\n");

	return fasync_helper(fd, file, on, &key_fasync);
}

struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = sixth_open,
	.read = sixth_read,
	.write = sixth_write,
	.release = sixth_release,
	.poll = sixth_poll,
	.fasync = sixth_fasync,		//异步通知

};

int major;
struct class 		*sixth_class;
struct class_device 	*sixth_class_device;

int sixth_init(void)
{
	major = register_chrdev(0, "sixth_drv",&f_ops);	//名字不重要 cat /proc/devices时显示252设备号和名字

	sixth_class = class_create(THIS_MODULE, "sixth_key_drv");

	sixth_class_device = class_device_create(sixth_class, NULL, MKDEV(major,0), NULL, "keys");

	gpgcon = (volatile unsigned long *)
				ioremap(0x56000060, sizeof(unsigned long));		//映射虚拟地址

	gpgdat = (volatile unsigned long *)
				ioremap(0x56000064, sizeof(unsigned long));


	return 0;
}

void sixth_exit(void)
{
	unregister_chrdev(major, "sixth_drv");       //名字不重要，"sec"也可以

	class_device_unregister(sixth_class_device);	//先unregister, 再destroy
	
	class_destroy(sixth_class);

	iounmap(gpgcon);
	iounmap(gpgdat);
}

module_init(sixth_init)
module_exit(sixth_exit)
MODULE_LICENSE("GPL");

