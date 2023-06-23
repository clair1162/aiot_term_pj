#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
//#include <pthread.h>
#include <wiringPi.h>


#define FIFO_FILE   "/tmp/fifo"
#define BUFF_SIZE   1024


const int FndSelectPin[6] = {4, 17, 18, 27, 22, 23};
const int FndPin[8] = {6, 12, 13, 16, 19, 20, 26, 21};
const int FndFont[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};
int pos[6] = {0, 0, 0, 0, 0, 0};
int timer = 0;
bool stop = false;


void Setup() {
  int i;
  //int fd;

  if (wiringPiSetupGpio() == -1) {
    printf("wiringPiSetupGpio() error\n");
    exit(-1);
  }

  for (i = 0; i < 6; i++) {
    pinMode(FndSelectPin[i], OUTPUT);
    digitalWrite(FndSelectPin[i], HIGH);
  }

  for (i = 0; i < 8; i++) {
    pinMode(FndPin[i], OUTPUT);
    digitalWrite(FndPin[i], LOW);
  }

  if (access(FIFO_FILE, F_OK) == -1) {
      mkfifo(FIFO_FILE, 0666);
  }

   stop = false;
}


void FndSelect(int position) {
  int i;

  for (i = 0; i < 6; i++) {
    if (i == position) {
      digitalWrite(FndSelectPin[i], LOW);
    }
    else {
      digitalWrite(FndSelectPin[i], HIGH);
    }
  }
}


void FndDisplay(int position, int num) {
  int i;
  int flag = 0;
  int shift = 0x01;

  FndSelect(position);

  for (i = 0; i < 8; i++) {
    if (position == 2)
      flag = ((FndFont[num] | 0x80) & shift);
    else
      flag = (FndFont[num] & shift);
    digitalWrite(FndPin[i], flag);
    shift <<= 1;
  }
}


void FndThread(void* arg) {
   while(1) {
      int i;
      int data[10]={0,1,2,3,4,5,6,7,8,9};


      for(i=0; i<6; i++) {
         FndDisplay(i, data[(timer/(int)pow(10,i))%10]);
         delay(1);
         if((i+timer)%2 == 0)
            delay(1);
      }

      delay(1);

      if(stop == true) { continue; }

      timer++;

      if(timer>=1000000) {
         timer = 0;
      }
   }
}


void IPCThread(void* arg) {
	int fd;
	char buff[BUFF_SIZE];
	
  fd = open(FIFO_FILE, O_RDONLY);

	while(1) {
		memset(buff, 0, BUFF_SIZE);
		read(fd, buff, BUFF_SIZE);

    if(strcmp(buff, "") != 0) {
      if(strcmp(buff, "2") == 0) {
        stop = true;
        exit(1);
        perror("stop error! : ");
      }

      else if(strcmp(buff, "3") == 0) {
        timer = 0;
        perror("clear error! : ");
      }
      
      // start
      else if(strcmp(buff, "1") == 0) {
        stop = false;
        perror("press start");
      }

      // start process exists?
      else if(strcmp(buff, "4") == 0) {
        const char *iamalive = "yes";

        fd = open(FIFO_FILE, O_WRONLY);
        write(fd, iamalive, strlen(iamalive));
        close(fd);
      }

      else {
        perror("read buff error! : ");
        exit(1);
      }
    }

    close(fd);
    delay(10);
	}
}


int main() {
  setbuf(stdout, NULL);

	printf("Content-type:text/html\n\n");
	printf("<html>\n<head>\n<title>STOP WATCH PROGRAM</title>\n</head>\n");printf("<body>\n<p>Stop Watch</p>\n</br>\n");
	printf("<form method=get action='num2.cgi'>\n");
	printf("<input type='submit' name='button' value='Start'>\n</form>\n");
	printf("<form method=get action='num_st.cgi'>\n");
	printf("<input type='submit' name='button' value='Stop'>\n</form>\n");
	printf("<form method=get action='num_cl.cgi'>\n");
	printf("<input type='submit' name='button' value='Clear'>\n</form>\n");

  pthread_t p_thread[2];
  int thr_id_fnd;
  int thr_id_ipc;
  int status;
  int a = 1;
  int b = 2;

  int pid;
  bool is_alive = false;
  int fd;
  char *status_ps = "4";
  char buff_ps[BUFF_SIZE];
  
  if(access(FIFO_FILE, F_OK) == -1) {
	  perror("fifo file not exist.");
  }
  else if(access(FIFO_FILE, F_OK) == 0){
    fd = open(FIFO_FILE, O_WRONLY);
	  write(fd, status_ps, strlen(status_ps));
    close(fd);

    delay(1000);

    fd = open(FIFO_FILE, O_RDONLY);
    memset(buff_ps, 0, BUFF_SIZE);
    read(fd, buff_ps, BUFF_SIZE);

    if(strcmp(buff_ps, "yes") == 0) {
        is_alive = true;
        perror("process start exist.");
    }
	
	  close(fd);
  }

  
  // 기존에 start process가 실행중이지 않으면
  if(!is_alive){
    pid = fork();
    if (pid == -1) {
        printf("Failed to fork.\n");
        return 1;
    }
    else if(pid == 0){
        thr_id_fnd = pthread_create(&p_thread[0], NULL, FndThread, NULL);
        if(thr_id_fnd < 0) {
            perror("thread create error: ");
            exit(1);
        }

        thr_id_ipc = pthread_create(&p_thread[1], NULL, IPCThread, NULL);
        if(thr_id_ipc <0) {
            perror("thread create error: ");
            exit(1);
        }

        pthread_join(p_thread[0], NULL);
        pthread_join(p_thread[1], NULL);
    }
    else{
        Setup();
    }
  }
  // start process가 있을 때 
  else{
    char *str = "1";

    fd = open(FIFO_FILE, O_WRONLY);
    write(fd, str, strlen(str));
    close(fd);
  }
	printf("</body>\n</html>");

	return 0;
}
