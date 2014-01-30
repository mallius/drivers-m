#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
	int fd;
	unsigned char buf;
	fd = open("/dev/keys", O_RDWR);

	if(fd < 0)
	{
		printf("Can not open.\n");
	}


	unsigned int ct = 0;
	while(1)
	{
		read(fd, &buf, 1);
		{
			printf("key_val:0x%x\n", buf);
		}
	}


	return 0;
}
