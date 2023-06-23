#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wiringPi.h>

#define FIFO_FILE	"/tmp/fifo"
#define BUFF_SIZE	1024

int main() {
	int fd;
	char *str = "3";

	fd = open(FIFO_FILE, O_WRONLY);

	write(fd, str, strlen(str));
	
	close(fd);

	return 0;
}

