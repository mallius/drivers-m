#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
	int fd;
	int buf;
	fd = open("/dev/xyz", O_RDWR);

	if(fd < 0)
	{
		printf("Can not open.\n");
	}

	if(argc != 2)
	{
		printf("Usage: %s on | off\n", argv[1]);
	}

	int val;

	if(strcmp(argv[1], "on") == 0)		//./test on
	{
		buf = 1;
	}
	else if(strcmp(argv[1], "off") == 0)	//./test off
	{
		buf = 0;
	}

	write(fd, &buf, 4);
	printf("argv[1]:%s\n", argv[1]);
	printf("buf:%d\n", buf);

	return 0;
}
