#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/plat-s3c24xx/ts.h>

#include <asm/arch/regs-adc.h>
#include <asm/arch/regs-gpio.h>

#include <asm/bitops.h>


struct input_dev *s3c_ts_dev;
struct clk *clk;
struct s3c_ts_regs {
	unsigned long adccon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
};
volatile struct s3c_ts_regs *s3c_ts_regs;

void enter_wait_pen_down_mode()
{
	s3c_ts_regs->adctsc = 0xd3;
}

void enter_wait_pen_up_mode()
{
	s3c_ts_regs->adctsc = 0x1d3;
}

irqreturn_t pen_down_up(int irq, void *dev_id)
{
	if(s3c_ts_regs->adcdat0 & (1<<15))
	{
		printk("pen up\n");
		enter_wait_pen_down_mode();
	}
	else
	{
		printk("pen down\n");
		enter_wait_pen_up_mode();
	}
	return IRQ_HANDLED;
}

int s3c_ts_init(void)
{
	/*1.分配input结构体*/
	s3c_ts_dev = input_allocate_device();
	if(!s3c_ts_dev)
	{
		printk("erro: input_allocate_device, no memory");
		return -ENOMEM;
	}


	/*2.设置*/
	/*2.1能产生哪类事件*/
	set_bit(EV_KEY, s3c_ts_dev);
	set_bit(EV_ABS, s3c_ts_dev);

	/*2.2能产生这类事件中的哪些事件*/
	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0X3FF, 0, 0);
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0X3FF, 0, 0);
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);


	


	/*3.注册*/
	input_register_device(s3c_ts_dev);

	/*4.硬件相关代码*/
	/*4.1使能时钟(CLKCON[15])*/
	clk = clk_get(NULL, "adc");

	clk_enable(clk);


	/*4.2设置s3c2440的ADC和TS寄存器*/
	s3c_ts_regs = ioremap(0x58000000, sizeof(struct s3c_ts_regs));

	/*see notes*/
	s3c_ts_regs->adccon = (1<<14)|(49<<6);
	
	request_irq(IRQ_TC, pen_down_up, IRQF_SAMPLE_RANDOM, "pen_ts", NULL);

	enter_wait_pen_down_mode();
	//enter_wait_pen_up_mode();


	return 0;
}

void s3c_ts_exit(void)
{
	free_irq(IRQ_TC, NULL);
	iounmap(s3c_ts_regs);
	input_unregister_device(s3c_ts_dev);
	input_free_device(s3c_ts_dev);
}

module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
