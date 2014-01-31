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

void my_signal_fun(int signum)
{
	read(fd, &key_val_buf, 1);
	printf("key val:%d\n", key_val_buf);
}

int main(int argc, char *argv[])
{
	unsigned char buf;
	int ret;
	int timeout = 2000;

	fd = open("/dev/keys", O_RDWR);
	if(fd < 0)
	{
		printf("Can not open.\n");
		return -1;
	}

	fcntl(fd, F_SETOWN, getpid());

	int oflags;
	oflags = fcntl(fd, F_GETFL);

	fcntl(fd, F_SETFL, oflags|FASYNC);

	signal(SIGIO, my_signal_fun);
	while(1)
	{
		sleep(1000);
	}

	return 0;
}
