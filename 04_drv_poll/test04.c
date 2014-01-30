#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <poll.h>  	//poll

int main(int argc, char *argv[])
{
	int fd;
	unsigned char buf;
	int ret;
	int timeout = 2000;
	struct pollfd fds;

	fd = open("/dev/keys", O_RDWR);

	if(fd < 0)
	{
		printf("Can not open.\n");
	}

	fds.fd = fd;
	fds.events = POLLIN;

	while(1)
	{
		ret = poll(&fds, 1, timeout);
		if(ret == 0)
		{
			printf("time out...\n");
		}
		else
		{
			read(fd, &buf, 1);
			printf("key_val:0x%x\n", buf);
		}
	}

	return 0;
}
