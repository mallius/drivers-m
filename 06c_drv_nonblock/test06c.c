#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <poll.h>  	//poll
#include <unistd.h>	//sleep()
#include <signal.h>     //signal()
#include <fcntl.h>	//fcntl()
#include <sys/types.h>  //getpid()

int fd;
unsigned char key_val_buf;

#if 0
void my_signal_fun(int signum)
{
	read(fd, &key_val_buf, 1);
	printf("key val:%d\n", key_val_buf);
}
#endif

int main(int argc, char *argv[])
{
	unsigned char buf;
	int ret;
	int timeout = 2000;

	fd = open("/dev/keys", O_RDWR | O_NONBLOCK);
	if(fd < 0)
	{
		printf("Can not open.\n");
		return -1;
	}

	while(1)
	{

		ret = read(fd, &key_val_buf, 1);
		printf("key val:%d\n ret:%d\n", key_val_buf, ret);

		sleep(5);
	}

	return 0;
}
