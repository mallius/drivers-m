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
#include <asm/io.h>
#include <linux/dma-mapping.h>


#include <linux/mm.h> /* need struct page */

#include <asm/scatterlist.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/interrupt.h>



static struct s3c_dma_regs {
	unsigned long disrc;
	unsigned long disrcc;
	unsigned long didst;
	unsigned long didstc;
	unsigned long dcon;
	unsigned long dstat;
	unsigned long dcsrc;
	unsigned long dcdst;
	unsigned long dmasktrig;
};
static struct class *cls;

static volatile struct s3c_dma_regs *s3c_dma_regs;
static int major;
static char *src;
static u32 src_phys;
static char *dst;
static u32 dst_phys;

#define MEM_CPY_NO_DMA 	0
#define MEM_CPY_DMA 	1
#define BUF_SIZE (512*1024)
#define DMA_BASE00 0X4B000000
#define DMA_BASE01 0X4B000040
#define DMA_BASE02 0X4B000080
#define DMA_BASE03 0X4B0000C0


static DECLARE_WAIT_QUEUE_HEAD(dma_wq);
static volatile int ev_dma = 0;

static int s3c_dma_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int i;
	memset(src, 0XAA, BUF_SIZE);
	memset(dst, 0X55, BUF_SIZE);
	switch(cmd)
	{
		case MEM_CPY_NO_DMA:
		{
			for(i = 0; i < BUF_SIZE; i++)
				dst[i] = src[i];
			if(memcmp(src, dst, BUF_SIZE) == 0)
				printk("MEM_CPY_NO_DMA OK.\n");
			else
				printk("MEM_CPY_NO_DMA ERROR.\n");
			break;
		}
		case MEM_CPY_DMA:
		{
			ev_dma = 0;

			s3c_dma_regs->disrc = src_phys;
			s3c_dma_regs->disrcc = ((0<<1) | (0<<0));
			s3c_dma_regs->dcdst = dst_phys;
			s3c_dma_regs->didstc = ((0<<2) | (0<<1) | (0<<0));
			s3c_dma_regs->dcon = ((1<<30) | (1<<29) | (0<<28) | (1<<27) | (0<<23) | (0<<20)|(BUF_SIZE<<0));
			s3c_dma_regs->dmasktrig = ((1<<1) | (1<<0));
			//休眠
			wait_event_interruptible(dma_wq, ev_dma);

			for(i = 0; i < BUF_SIZE; i++)
				dst[i] = src[i];
			if(memcmp(src, dst, BUF_SIZE) == 0)
				printk("MEM_CPY_DMA OK.\n");
			else
				printk("MEM_CPY_DMA ERROR.\n");

			break;
		}
		
	}
}

static struct file_operations dma_fops = {
	.owner = THIS_MODULE,
	.ioctl = s3c_dma_ioctl,
};

static irqreturn_t s3c_dma_handler(int irq, void *devid)
{
	ev_dma = 1;
	wake_up_interruptible(&dma_wq);

	return IRQ_HANDLED;
}

static int s3c_dma_init(void)
{
	if(request_irq(IRQ_DMA3, s3c_dma_handler, 0, "s3c_dma", 1))
	{
		printk("erro: can not request irq.\n");
		return -EBUSY;
	}
	//分配src，dst对应的缓冲区
	src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
	if(NULL == src)
	{
		printk("can not alloc buf for src");
		free_irq(IRQ_DMA3, 1);

		return -ENOMEM;
	}

	dst = dma_alloc_writecombine(NULL, BUF_SIZE, &dst_phys, GFP_KERNEL);
	if(NULL == dst)
	{
		dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
		free_irq(IRQ_DMA3, 1);
		printk("can not alloc buf for dst");
		return -ENOMEM;
	}

	major = register_chrdev(0, "s3c_dma", &dma_fops);

	cls = class_create(THIS_MODULE, "s3c_dma");
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "dma");

	s3c_dma_regs = ioremap(DMA_BASE03, sizeof(struct s3c_dma_regs));

	return 0;
}

static void s3c_dma_exit(void)
{
	iounmap(s3c_dma_regs);
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "s3c_dma");
	dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
	dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
	free_irq(IRQ_DMA3, 1);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);
MODULE_LICENSE("GPL");
