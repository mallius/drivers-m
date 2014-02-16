#include <linux/module.h>     //s3c2410fb.c --> *.h
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include <asm/plat-s3c24xx/s3c2410.h>

struct fb_info *s3c_lcd;
int lcd_init(void)
{
	/*1.分配一个fb_info*/
	s3c_lcd = framebuffer_alloc(0, NULL);
	if(!s3c_lcd)
	{
		printk("framebuffer_alloc: error.\n");
		return -1;
	}

	/*2.设置*/

	/*2.1设置固定的参数*/

	/*2.2设置可变的参数*/

	/*2.3设置操作的函数*/

	/*2.4其他的设置*/




	/*3.硬件相关的操作*/

	/*3.1配置GPIO用于LCD*/

	/*3.2设置LCD控制器*/

	/*3.3分配framebuffer并把地址告诉LCD控制器*/





	/*4.注册*/
	register_framebuffer(s3c_lcd);

	return 0;
}

void lcd_exit(void)
{

}
module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");
