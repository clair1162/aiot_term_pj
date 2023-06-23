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
	printf("<body>\n<p>Stop Watch</p>\n</br>\n");
	printf("<form method=get action='chae.cgi'>\n");
	printf("<input type='submit' name='button' value='Start'>\n</form>\n");
	printf("<form method=get action='num_st.cgi'>\n");
	printf("<input type='submit' name='button' value='Stop'>\n</form>\n");
	printf("<form method=get action='num_cl.cgi'>\n");
	printf("<input type='submit' name='button' value='Clear'>\n</form>\n");
	printf("</body>\n</html>");

	int fd;
	char *str = "2";

	fd = open(FIFO_FILE, O_WRONLY);

	write(fd, str, strlen(str));
	
	close(fd);

	return 0;
}
