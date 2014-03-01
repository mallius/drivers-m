#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>


static unsigned short ignore[] = { I2C_CLIENT_END };
static unsigned short normal_addr[] = { 0x50, I2C_CLIENT_END };

static unsigned short force_addr[] = { ANY_I2C_BUS, 0x60, I2C_CLIENT_END };
static unsigned short *forces[] = { force_addr, NULL };

static struct i2c_client_address_data addr_data = {
	.normal_i2c	= normal_addr,
	.probe		= ignore,
	.ignore		= ignore,
	//.forces 	= forces,
};

static struct i2c_driver at24cxx_driver;
struct i2c_client *new_client;

static int at24cxx_detect(struct i2c_adapter *adapter, int address, int kind)
{

	printk("at24cxx_detect.\n");
	/*构造一个i2c_client结构体*/
	new_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);

	new_client->addr = address;
	new_client->adapter = adapter;
	new_client->driver = &at24cxx_driver;
	strcpy(new_client->name, "at24cxx");

	i2c_attach_client(new_client);

	return 0;
}

static int at24cxx_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, at24cxx_detect);
}


static int at24cxx_detach(struct i2c_client *client)
{
	printk("at24cxx_detach.\n");

	i2c_detach_client(new_client);

	kfree(i2c_get_clientdata(new_client));
	return 0;
}

/*1.分配一个i2c_driver结构体*/
/*2.设置*/

static struct i2c_driver at24cxx_driver = {
	.driver = {
		.name	= "at24cxx",
	},
	//.id = I2C_DRIVERID_DS1374,
	.attach_adapter = at24cxx_attach,
	.detach_client = at24cxx_detach,
};
static int at24cxx_init(void)
{
	i2c_add_driver(&at24cxx_driver);
	return 0;
}

static void at24cxx_exit(void)
{
	i2c_del_driver(&at24cxx_driver);
}

module_init(at24cxx_init);
module_exit(at24cxx_exit);

MODULE_LICENSE("GPL");
