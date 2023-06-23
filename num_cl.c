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
	printf("Content-type:text/html\n\n");
	printf("<html>\n<head>\n<title>STOP WATCH!</title>\n</head>\n");
	printf("<body>\n<p>Clear</p>\n");

	int fd;
	char *str = "3";

	fd = open(FIFO_FILE, O_WRONLY);

	write(fd, str, strlen(str));
	
	close(fd);

	printf("</body>\n</html>");

	return 0;
}

