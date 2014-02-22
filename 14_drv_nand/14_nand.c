





#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>

#include <asm/arch/regs-nand.h>
#include <asm/arch/nand.h>

struct s3c_nand_regs {
	unsigned long nfconf;
	unsigned long nfcont;
	unsigned long nfcmmd;
	unsigned long nfaddr;
	unsigned long nfdata;
	unsigned long nfmeccd0;
	unsigned long nfmeccd1;
	unsigned long nfseccd;
	unsigned long nfstat;
	unsigned long nfestat0;
	unsigned long nfestat1;
	unsigned long nfmecc0;
	unsigned long nfmecc1;
	unsigned long nfsecc;
	unsigned long nfsblk;
	unsigned long nfeblk;
};


static struct nand_chip *s3c_nand;
static struct mtd_info *s3c_mtd;
static struct s3c_nand_regs *s3c_nand_regs;


static struct mtd_partition s3c_nand_parts[] = {
	[0] = {
        .name   = "bootloader",
        .size   = 0x00040000,
		.offset	= 0,
	},
	[1] = {
        .name   = "params",
        .offset = MTDPART_OFS_APPEND,
        .size   = 0x00020000,
	},
	[2] = {
        .name   = "kernel",
        .offset = MTDPART_OFS_APPEND,
        .size   = 0x00200000,
	},
	[3] = {
        .name   = "root",
        .offset = MTDPART_OFS_APPEND,
        .size   = MTDPART_SIZ_FULL,
	}
};


static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{
		/*取消选中*/
		s3c_nand_regs->nfcont |= (1<<0);
	}
	else
	{
		/*选中*/
		s3c_nand_regs->nfcont &= ~(1<<0);
	}
}

static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{
		/*发命令：NFCMD=dat*/
		s3c_nand_regs->nfcmmd = dat;
	}
	else
	{
		/*发地址: NFADDR=dat*/
		s3c_nand_regs->nfaddr = dat;
	}
}

static int s3c2440_dev_ready(struct mtd_info *mtd)
{
	return (s3c_nand_regs->nfstat & (1<<0));
}


#define DBG printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__)
//#define DBG (x...)
static int s3c_nand_init(void)
{
	struct clk *clk;

	/*1分配一个nand_chip结构体*/
	s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	if(s3c_nand == NULL) printk("erro: kzalloc nand_chip.\n");
	s3c_nand_regs = ioremap( 0x4E000000, sizeof(struct s3c_nand_regs));

	DBG;


	/*2设置*/
	s3c_nand->select_chip = s3c2440_select_chip;
	s3c_nand->cmd_ctrl    = s3c2440_cmd_ctrl;
	s3c_nand->IO_ADDR_R = &s3c_nand_regs->nfdata ;
	s3c_nand->IO_ADDR_W = &s3c_nand_regs->nfdata;
	s3c_nand->dev_ready = s3c2440_dev_ready;
	s3c_nand->ecc.mode = NAND_ECC_SOFT;


	/*3硬件相关的设置*/
	clk = clk_get(NULL, "nand");
	clk_enable(clk);




#define TACLS 0
#define TWRPH0 1
#define TWRPH1 0
	s3c_nand_regs->nfcont = (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4);


	DBG;


	s3c_nand_regs->nfcont = (1<<1) | (1<<0);

	/*4使用：nand_scan*/
	s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if(s3c_mtd != NULL) printk("succes: kzalloc mtd_info.\n");
	s3c_mtd->owner = THIS_MODULE;
	s3c_mtd->priv = s3c_nand;
	DBG;
	nand_scan(s3c_mtd, 1);
	DBG;
	/*5 add_mtd_partitions*/
	add_mtd_partitions(s3c_mtd, s3c_nand_parts, 4);
	DBG;

	return 0;
}

static void s3c_nand_exit(void)
{
	del_mtd_partitions(s3c_mtd);
	kfree(s3c_mtd);
	iounmap(s3c_nand_regs);
	kfree(s3c_nand);
}

module_init(s3c_nand_init);
module_exit(s3c_nand_exit);

MODULE_LICENSE("GPL");
