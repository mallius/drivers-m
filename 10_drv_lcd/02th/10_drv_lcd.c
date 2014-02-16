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

struct fb_info *s3c_lcd;
#if 0
struct fb_fix_screeninfo {
	char id[16];			/* identification string eg "TT Builtin" */
	unsigned long smem_start;	/* Start of frame buffer mem */
					/* (physical address) */
	__u32 smem_len;			/* Length of frame buffer mem */
	__u32 type;			/* see FB_TYPE_*		*/
	__u32 type_aux;			/* Interleave for interleaved Planes */
	__u32 visual;			/* see FB_VISUAL_*		*/ 
	__u16 xpanstep;			/* zero if no hardware panning  */
	__u16 ypanstep;			/* zero if no hardware panning  */
	__u16 ywrapstep;		/* zero if no hardware ywrap    */
	__u32 line_length;		/* length of a line in bytes    */
	unsigned long mmio_start;	/* Start of Memory Mapped I/O   */
					/* (physical address) */
	__u32 mmio_len;			/* Length of Memory Mapped I/O  */
	__u32 accel;			/* Indicate to driver which	*/
					/*  specific chip/card we have	*/
	__u16 reserved[3];		/* Reserved for future compatibility */
};
#endif

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




#if 0
struct fb_var_screeninfo {
	__u32 xres;			/* visible resolution		*/
	__u32 yres;
	__u32 xres_virtual;		/* virtual resolution		*/
	__u32 yres_virtual;
	__u32 xoffset;			/* offset from virtual to visible */
	__u32 yoffset;			/* resolution			*/

	__u32 bits_per_pixel;		/* guess what			*/
	__u32 grayscale;		/* != 0 Graylevels instead of colors */

	struct fb_bitfield red;		/* bitfield in fb mem if true color, */
	struct fb_bitfield green;	/* else only length is significant */
	struct fb_bitfield blue;
	struct fb_bitfield transp;	/* transparency			*/	

	__u32 nonstd;			/* != 0 Non standard pixel format */

	__u32 activate;			/* see FB_ACTIVATE_*		*/

	__u32 height;			/* height of picture in mm    */
	__u32 width;			/* width of picture in mm     */

	__u32 accel_flags;		/* (OBSOLETE) see fb_info.flags */

	/* Timing: All values in pixclocks, except pixclock (of course) */
	__u32 pixclock;			/* pixel clock in ps (pico seconds) */
	__u32 left_margin;		/* time from sync to picture	*/
	__u32 right_margin;		/* time from picture to sync	*/
	__u32 upper_margin;		/* time from sync to picture	*/
	__u32 lower_margin;
	__u32 hsync_len;		/* length of horizontal sync	*/
	__u32 vsync_len;		/* length of vertical sync	*/
	__u32 sync;			/* see FB_SYNC_*		*/
	__u32 vmode;			/* see FB_VMODE_*		*/
	__u32 rotate;			/* angle we rotate counter clockwise */
	__u32 reserved[5];		/* Reserved for future compatibility */
};

#endif
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
