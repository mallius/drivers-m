#include <linux/device.h>   //class_create()
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
#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/gpio_keys.h>

#include <asm/gpio.h>
#include <asm/bitops.h>
#include <linux/timer.h>

struct pin_des{
	int irq;
	unsigned int pin;
	char *name;
	unsigned int pin_val;
};

struct pin_des pins[4] = {
	{IRQ_EINT8, S3C2410_GPG0, "K1", KEY_L},
	{IRQ_EINT11, S3C2410_GPG3, "K2", KEY_S},
	{IRQ_EINT13, S3C2410_GPG5, "K3", KEY_ENTER},
	{IRQ_EINT14, S3C2410_GPG6, "K4", KEY_LEFTSHIFT},
};


struct input_dev *buttons_dev;

struct pin_des *pin_ptr;
struct timer_list buttons_timer;

irqreturn_t buttons_irq_handler(int irq, void *dev_id)
{
	pin_ptr = (struct pin_des *)dev_id;

	mod_timer(&buttons_timer, jiffies+HZ/100);
	return IRQ_HANDLED;
}

void buttons_timer_function(void)
{
	/*
	 *读出引脚值，是按下或是松开,按下引脚是低电平，松开引脚是高电平
	 */
	unsigned int pinval;
	pinval = s3c2410_gpio_getpin(pin_ptr->pin);

	struct pin_des *ptr = pin_ptr;
	if(!ptr)
		return ;

	if(pinval)
	{

		/*最后一个参数：1表示按下，0表示松开*/
		input_event(buttons_dev, EV_KEY, pin_ptr->pin_val, 0);
		input_sync(buttons_dev);
	}
	else
	{
		/*按下,低电平*/
		input_event(buttons_dev, EV_KEY, pin_ptr->pin_val, 1);
		input_sync(buttons_dev);
	}
}

static int buttons_init(void)
{
	/*1.分配一个input_dev结构体*/
	buttons_dev = input_allocate_device();
	if(!buttons_dev)
	{
		printk("input_dev == NULL\n");
		input_free_device(buttons_dev);
	}

	/*2.设置*/
	/*
	unsigned long evbit[NBITS(EV_MAX)];
	unsigned long keybit[NBITS(KEY_MAX)];
	unsigned long relbit[NBITS(REL_MAX)];
	unsigned long absbit[NBITS(ABS_MAX)];
	unsigned long mscbit[NBITS(MSC_MAX)];
	unsigned long ledbit[NBITS(LED_MAX)];
	unsigned long sndbit[NBITS(SND_MAX)];
	unsigned long ffbit[NBITS(FF_MAX)];
	unsigned long swbit[NBITS(SW_MAX)];
	*/
	set_bit(EV_KEY, buttons_dev->evbit);
	set_bit(EV_REP, buttons_dev->evbit);


	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER, buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

	/*3.注册*/
	input_register_device(buttons_dev);

	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer);
	
	/*4.硬件相关*/
	int i;
	for(i = 0; i<4; i++)
	{
		request_irq(pins[i].irq, buttons_irq_handler, IRQT_BOTHEDGE, 
								pins[i].name, &pins[i]);
		
	}
	return 0;
}


static void buttons_exit(void)
{
	int i;
	for(i = 0; i<4; i++)
	{
		free_irq(pins[i].irq, &pins[i]);
	}

	del_timer(&buttons_timer);
	input_unregister_device(buttons_dev);
	input_free_device(buttons_dev );
}

module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");
