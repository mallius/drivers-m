#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

struct gendisk *ramblock_disk;
request_queue_t *ramblock_queue;

static DEFINE_SPINLOCK(ramblock_lock);

static int major;

static struct block_device_operations ramblock_fops = {
	.owner	= THIS_MODULE,
};

#define RAMBLOCK_SIZE (1024*1024)


static void do_ramblock_request (request_queue_t * q)
{
	static int cnt = 0;
	struct request *req;

	printk("do_ramblock_queue: %d\n", ++cnt);
	
	while ((req = elv_next_request(q)) != NULL)
       	{
		end_request(req, 1);	/* wrap up, 0 = fail, 1 = success */
	}
}

static int ramblock_init(void)
{

	/*1.分配gendisk结构体*/
	ramblock_disk = alloc_disk(16);		//次设备号个数

	/*2设置*/
	/*2.1分配设置队列*/
	ramblock_queue =  blk_init_queue(do_ramblock_request, &ramblock_lock);
	ramblock_disk->queue = ramblock_queue;

	/*2.2设置其它属性*/
	major = register_blkdev(0, "ramblock");
		
	ramblock_disk->major = major;
	ramblock_disk->first_minor = 0;
	sprintf(ramblock_disk->disk_name, "ramblock");
	ramblock_disk->fops = &ramblock_fops;

	set_capacity(ramblock_disk, RAMBLOCK_SIZE / 512);


	/*3注册*/
	add_disk(ramblock_disk);

	return 0;
}

static void ramblock_exit(void)
{
    unregister_blkdev( major, "ramblock");

    del_gendisk(ramblock_disk);
    put_disk(ramblock_disk);
    blk_cleanup_queue(ramblock_queue);
}

module_init(ramblock_init);
module_exit(ramblock_exit);
MODULE_LICENSE("GPL");

