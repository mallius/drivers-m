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


#include <linux/mm.h> /* need struct page */

#include <asm/scatterlist.h>


static int major;
#define MEM_CPY_NO_DMA 	0
#define MEM_CPY_DMA 	1

static char *src;
static u32 src_phys;

static char *dst;
static u32 dst_phys;

#define BUF_SIZE (512*1024)
static struct class *cls;

static int s3c_dma_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case MEM_CPY_NO_DMA:
		{
			break;
		}
		case MEM_CPY_DMA:
		{
			break;
		}
		
	}
}

static struct file_operations dma_fops = {
	.owner = THIS_MODULE,
	.ioctl = s3c_dma_ioctl,
};

static int s3c_dma_init(void)
{
	//分配src，dst对应的缓冲区
	src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
	if(NULL == src)
	{
		printk("can not alloc buf for src");
		return -ENOMEM;
	}

	dst = dma_alloc_writecombine(NULL, BUF_SIZE, &dst_phys, GFP_KERNEL);
	if(NULL == dst)
	{
		dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
		printk("can not alloc buf for dst");
		return -ENOMEM;
	}

	major = register_chrdev(0, "s3c_dma", &dma_fops);

	cls = class_create(THIS_MODULE, "s3c_dma");
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "dma");
	return 0;
}

static void s3c_dma_exit(void)
{
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "s3c_dma");
	dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
	dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);
MODULE_LICENSE("GPL");
