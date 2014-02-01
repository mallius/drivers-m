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

#include <asm-generic/errno-base.h>	//-EBUSY

#include <asm/semaphore.h>		//DECLARE_MUTEX, down(), up()

#include <linux/timer.h>
#include <asm/param.h>			//HZ

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
atomic_t canopen = ATOMIC_INIT(1);		//原子操作
static DECLARE_MUTEX(key_lock);

struct timer_list key_timer;
struct pin_des *pin_ptr ;

irqreturn_t key_handler(int irq, void *dev_id)
{
	pin_ptr = (struct pin_des *)dev_id;

	mod_timer(&key_timer, jiffies+HZ/100);
	return IRQ_HANDLED;
}

int seventh_open (struct inode *inode, struct file *file)
{
	printk("seventh_open: \n");

	/* K1,K2,K3,K4--->EINT8,EINT11,EINT13,EINT14 */



	if(file->f_flags & O_NONBLOCK)
	{
		if(down_trylock(&key_lock)) return -EBUSY;
	}
	else
	{
		down(&key_lock);
	}

	request_irq(IRQ_EINT8, key_handler, IRQT_BOTHEDGE, "K1", &pins[0]);
	request_irq(IRQ_EINT11, key_handler, IRQT_BOTHEDGE, "K2", &pins[1]);
	request_irq(IRQ_EINT13, key_handler, IRQT_BOTHEDGE, "K3", &pins[2]);
	request_irq(IRQ_EINT14, key_handler, IRQT_BOTHEDGE, "K4", &pins[3]);

	return 0;
}


ssize_t seventh_read (struct file *file, char __user *user, size_t size, loff_t *loff)
{
//	printk("seventh_read:\n");
	
	if(size != 1)
	{
		printk("size != 1\n");
		return -EINVAL;
	}

	if(file->f_flags & O_NONBLOCK)
       	{
		if(!ev_press) 
			return -EAGAIN;
	}
       	else
	{
		wait_event_interruptible(key_waitirq, ev_press); 
		//if ev_press == 0, 休眠，ev_press ==1,不休眠
	}
							 //继续往下

	copy_to_user(user, &keyval, 1);
	ev_press = 0;

	return 1;

}

ssize_t seventh_write (struct file *file, const char __user *user, size_t size, loff_t *loff)
{
	printk("This is seventh_write.\n");

	return 0;
}

int seventh_release (struct inode *inode, struct file *file)
{

	//atomic_inc(&canopen);
	//up(&key_lock);

	free_irq(IRQ_EINT8, &pins[0]);
	free_irq(IRQ_EINT11, &pins[1]);
	free_irq(IRQ_EINT13, &pins[2]);
	free_irq(IRQ_EINT14, &pins[3]);

	up(&key_lock);

	return 0;
}

static unsigned int seventh_poll(struct file *file, poll_table *wait)
{

	unsigned int mask = 0;

	poll_wait(file, &key_waitirq, wait);

	
	if(ev_press)			//ev_press == 1 --> 有按键按下
	{
		mask |= POLLIN | POLLRDNORM ;
	}

	return mask;
}



int seventh_fasync(int fd, struct file *file, int on)
{
	printk("seventh_fasync:\n");

	return fasync_helper(fd, file, on, &key_fasync);
}

struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = seventh_open,
	.read = seventh_read,
	.write = seventh_write,
	.release = seventh_release,
	.poll = seventh_poll,
	.fasync = seventh_fasync,		//异步通知

};

int major;
struct class 		*seventh_class;
struct class_device 	*seventh_class_device;

void key_timer_fun(unsigned long data)
{
        /* 中断处理函数放到这里 */
	

	unsigned int pinval;

	/*
	 *读出引脚值，是按下或是松开,按下引脚是低电平，松开引脚是高电平
	 */
//	printk("key pressed..irq:%d\n", irq);

	pinval = s3c2410_gpio_getpin(pin_ptr->pin);
	struct pin_des *ptr = pin_ptr;

	if(!ptr)
		return ;

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

}

int seventh_init(void)
{
	init_timer(&key_timer);				//初始化计时器
	key_timer.function = key_timer_fun;
	add_timer(&key_timer);

	major = register_chrdev(0, "seventh_drv",&f_ops);	//名字不重要 cat /proc/devices时显示252设备号和名字

	seventh_class = class_create(THIS_MODULE, "seventh_key_drv");

	seventh_class_device = class_device_create(seventh_class, NULL, MKDEV(major,0), NULL, "keys");

	gpgcon = (volatile unsigned long *)
				ioremap(0x56000060, sizeof(unsigned long));		//映射虚拟地址

	gpgdat = (volatile unsigned long *)
				ioremap(0x56000064, sizeof(unsigned long));


	return 0;
}

void seventh_exit(void)
{
	unregister_chrdev(major, "seventh_drv");       //名字不重要，"sec"也可以

	class_device_unregister(seventh_class_device);	//先unregister, 再destroy
	
	class_destroy(seventh_class);

	iounmap(gpgcon);
	iounmap(gpgdat);
}

module_init(seventh_init)
module_exit(seventh_exit)
MODULE_LICENSE("GPL");

