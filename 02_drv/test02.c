#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
	int fd;
	unsigned char buf[4];
	fd = open("/dev/keys", O_RDWR);

	if(fd < 0)
	{
		printf("Can not open.\n");
	}


	unsigned int ct = 0;
	while(1)
	{
		read(fd, buf, sizeof(buf));
		if(buf[3] == 0 || buf[2] == 0 || buf[1] == 0 || buf[0] == 0)
		{
			printf("counts:%d %d %d %d %d\n", ct, buf[0], buf[1], buf[2], buf[3]);
			ct++;
		}
	}


	return 0;
}
