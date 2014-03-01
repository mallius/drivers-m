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
static int major;
struct i2c_client *at24cxx_client;


ssize_t at24cxx_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	unsigned char address;
	unsigned char data;
	struct i2c_msg msg[2];
	int ret;

	//addr = buf[0]
	//data = buf[1]
	if(size != 1)
	{
		return -EINVAL;
	}
	copy_from_user(&address, buf, 1);
	/*数据传输三要素：源，目的，长度*/
	//读AT24CXX时，要先把读的地址发给它
	msg[0].addr = at24cxx_client->addr;	//目的
	msg[0].buf = &address;			//源
	msg[0].len = 1;				//源+目的 = 2
	msg[0].flags = 0;			//表示写
	
	//然后启动读操作
	msg[1].addr = at24cxx_client->addr;	//源
	msg[1].buf = &data;			//目的
	msg[1].len = 1;				//源+目的 = 2
	msg[1].flags = I2C_M_RD;		//表示读


	ret = i2c_transfer(at24cxx_client->adapter, msg, 2);
	if(ret == 2)
	{
		copy_to_user(buf, &data, 1);
		return 1;
	}
	else
		return -EIO;

	return 0;
}

ssize_t at24cxx_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	unsigned char val[2];
	struct i2c_msg msg[1];
	int ret;

	//addr = buf[0]
	//data = buf[1]
	if(size != 2)
	{
		return -EINVAL;
	}
	copy_from_user(val, buf, 2);
	/*数据传输三要素：源，目的，长度*/
	msg[0].addr = at24cxx_client->addr;	//目的
	msg[0].buf = val;			//源
	msg[0].len = 2;				//源+目的 = 2
	msg[0].flags = 0;			//表示写

	ret = i2c_transfer(at24cxx_client->adapter, msg, 1);
	if(ret == 1)
		return 2;
	else
		return -EIO;
}


struct file_operations at24cxx_fops = {
	.owner = THIS_MODULE,
	.read = at24cxx_read,
	.write = at24cxx_write,
};

static struct class *cls;

static int at24cxx_detect(struct i2c_adapter *adapter, int address, int kind)
{

	printk("at24cxx_detect.\n");
	/*构造一个i2c_client结构体*/
	at24cxx_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);

	at24cxx_client->addr = address;
	at24cxx_client->adapter = adapter;
	at24cxx_client->driver = &at24cxx_driver;
	strcpy(at24cxx_client->name, "at24cxx");

	i2c_attach_client(at24cxx_client);

	major = register_chrdev(0, "at24cxx", &at24cxx_fops);
	cls = class_create(THIS_MODULE, "at24cxx");
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "at24cxx");

	return 0;
}

static int at24cxx_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, at24cxx_detect);
}


static int at24cxx_detach(struct i2c_client *client)
{
	printk("at24cxx_detach.\n");
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "at24cxx");

	i2c_detach_client(at24cxx_client);

	kfree(i2c_get_clientdata(at24cxx_client));
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
