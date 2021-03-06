#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <asm/io.h>


struct map_info *s3c_nor_map;
struct mtd_info *s3c_nor_mtd;
int s3c_nor_init(void)
{
	/*1.分配一个map_info结构体*/
	s3c_nor_map = kzalloc(sizeof(struct map_info), GFP_KERNEL);

	/*2.设置：物理基地址，大小，位宽，虚拟基地址*/

	s3c_nor_map->name = "s3c_nor";
	s3c_nor_map->phys = 0;
	s3c_nor_map->size = 0x1000000;
	s3c_nor_map->bankwidth = 2;

	s3c_nor_map->virt = ioremap(s3c_nor_map->phys, s3c_nor_map->size);
	simple_map_init(s3c_nor_map);

	/*3.使用：调用nor协议层提供的函数来识别*/

	printk("use cfi_probe");
	s3c_nor_mtd = do_map_probe("cfi_probe", s3c_nor_map);
	if(!s3c_nor_mtd)
	{
		s3c_nor_mtd = do_map_probe("jedec_probe", s3c_nor_map);
	}
	if(!s3c_nor_mtd)
	{
		printk("!s3c_nor_mtd.\n");
		iounmap(s3c_nor_map->virt);
		kfree(s3c_nor_map);
		return -1;
	}

	/*4.add_mtd_partitions*/
	return 0;
}

void s3c_nor_exit(void)
{
	iounmap(s3c_nor_map->virt);
	kfree(s3c_nor_map);
}

module_init(s3c_nor_init);
module_exit(s3c_nor_exit);
MODULE_LICENSE("GPL");

