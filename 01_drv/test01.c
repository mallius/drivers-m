#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


int main(void)
{
	int fd;
	int buf;
	fd = open("/dev/xyz", O_RDWR);
	if(fd < 0)
	{
		printf("Can not open.\n");
	}
	read(fd, &buf, 1);



	return 0;
}
