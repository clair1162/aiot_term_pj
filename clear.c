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

	if(-1 ==(fd = open(FIFO_FILE, O_WRONLY))) {
		perror("open() error!");
	}

	write(fd, str, strlen(str));
	close(fd);

	return 0;
}

