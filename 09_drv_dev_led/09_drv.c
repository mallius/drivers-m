#include <linux/platform_device.h>
#include <linux/kernel.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <asm/arch/regs-serial.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-lcd.h>


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

#include<asm/uaccess.h>

static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
int pin;

int led_open (struct inode *inode, struct file *file)
{

	printk("led_open: gpio_con output\n");

	/* GPB5,6,7,8 分别接LED01，LED02，LED03，LED04，配置成输出*/

	*gpio_con &= ~(0x3<<pin*2);		//
	*gpio_con |= (0x1<<pin*2);		//

	return 0;
}

ssize_t led_read (struct file *file, char __user *user, size_t size, loff_t *offset)
{
	return 0;
}

int val;
ssize_t led_write (struct file *file, const char __user *user, size_t size, loff_t *offset)
{

	printk("This is first_write.\n");


	copy_from_user(&val, user, 4);
	if(val == 1)
	{
		//点灯
		printk("led on.val:%d\n", val);
		*gpio_dat &= ~(0x1<<pin);				//亮LED1-->gpio_dat[5]=0
	}
	else if(val == 0)
	{
		//灭灯
		printk("led off.val:%d\n", val);
		*gpio_dat |= (0x1<<pin); 				//灭LED1
	}
}

struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
};

struct class *cls;
struct class_device *cls_dev;
int major;

int led_probe(struct platform_device *pdev)
{
	struct resource *res;

        res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio_con = ioremap(res->start, res->end - res->start + 1);
	gpio_dat = gpio_con + 1;

        res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	pin = res->start;
	



	printk("led_probe:\n");
	major = register_chrdev(0, "myled", &fops);
	cls = class_create(THIS_MODULE, "myled");
	
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "led");

	

	return 0;
}

int led_remove(struct platform_device *pdev)
{
	printk("led_remove:\n");

	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "myled");
	iounmap(gpio_con);

	return 0;
}

struct platform_driver led_driver = {
	.probe = led_probe,
	.remove = led_remove,
	.driver = {.name = "myled"},
};

int led_drv_init(void)
{
	platform_driver_register(&led_driver);
	return 0;
}

void led_drv_exit(void)
{
	platform_driver_unregister(&led_driver);
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");

