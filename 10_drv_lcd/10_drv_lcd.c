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

static struct fb_ops s3c_lcd_ops = {
	.owner		= THIS_MODULE,
	//.fb_setcolreg	= sa1100fb_setcolreg,

	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,

};

struct lcd_regs {
	unsigned long lcdcon1;		//0X4D000000
	unsigned long lcdcon2;
	unsigned long lcdcon3;
	unsigned long lcdcon4;
	unsigned long lcdcon5;
	unsigned long lcdsaddr1;
	unsigned long lcdsaddr2;
	unsigned long lcdsaddr3;
	unsigned long redlut;
	unsigned long greenlut;
	unsigned long bluelut;

	unsigned long reserved[9];

	unsigned long dithmode;
	unsigned long tpal;
	unsigned long lcdintpnd;
	unsigned long lcdsrcpnd;
	unsigned long lcdintmsk;
	unsigned long tconsel;
};

struct fb_info *s3c_lcd;
volatile unsigned long *gpccon;
volatile unsigned long *gpcdat;
volatile unsigned long *gpdcon;
volatile unsigned long *gpddat;
volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;
volatile struct lcd_regs *lcd_regs;

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
	strcpy(s3c_lcd->fix.id, "mylcd");
	s3c_lcd->fix.smem_len = 240*320*16;
	s3c_lcd->fix.type =  FB_TYPE_PACKED_PIXELS;
	s3c_lcd->fix.visual = FB_VISUAL_TRUECOLOR;
	s3c_lcd->fix.line_length = 240*2;

	/*2.2设置可变的参数*/
	s3c_lcd->var.xres = 240;
	s3c_lcd->var.yres = 320;
	s3c_lcd->var.xres_virtual = 240;
	s3c_lcd->var.yres_virtual = 320;

	s3c_lcd->var.bits_per_pixel = 16;
	s3c_lcd->var.red.offset = 11;
	s3c_lcd->var.red.length = 5;

	s3c_lcd->var.green.offset = 5;
	s3c_lcd->var.green.length = 6;

	s3c_lcd->var.blue.offset = 0;
	s3c_lcd->var.blue.length = 5;

	s3c_lcd->var.activate = FB_ACTIVATE_NOW;



	

	/*2.3设置操作的函数*/

	s3c_lcd->fbops = &s3c_lcd_ops;

	/*2.4其他的设置*/
	//s3c_lcd->pseudo_palette = ;
	//s3c_lcd->screen_base = ;
	s3c_lcd->screen_size = 240*324*16/8;



	/*3.硬件相关的操作*/

	/*3.1配置GPIO用于LCD*/
	gpccon = ioremap(0x56000020, 8);
	gpcdat = ioremap(0x56000024, 8);

	gpdcon = ioremap(0x56000030, 8);
	gpddat = ioremap(0x56000034, 8);

	gpgcon = ioremap(0x56000060, 8);
	gpgdat = ioremap(0x56000064, 8);

	*gpccon = 0xaaaaaaaa;		//全部10101010...
	*gpdcon = 0xaaaaaaaa;		//同上
	*gpgcon |= (0x3<<8);			//gpg4->LCDPWREN

	/*3.2设置LCD控制器*/

	lcd_regs = ioremap(0x4d000000, sizeof(struct lcd_regs));
	lcd_regs->lcdcon1 = (6<<8) | (3<<5) | (0x0c << 1);






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
