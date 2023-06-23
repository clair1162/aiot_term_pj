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


void Init() {
  int i;
  int fd;

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

  //************************
  //****** ADD - 3:45 ******
  //************************
  mkfifo(FIFO_FILE, 0666);
  //fd = fopen(FIFO_FILE, O_RDONLY);
  //close(fd);

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
  int i, j;
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
  usleep(100);

	int fd;
	char buff[BUFF_SIZE];
	
  fd = open(FIFO_FILE, O_RDONLY);

	while(1) {
		memset(buff, 0, BUFF_SIZE);

		read(fd, buff, BUFF_SIZE);
		if(strcmp(buff, "2") == 0) {	
			stop = true;
		}
		
		else if(strcmp(buff, "3") == 0) {
			timer = 0;
		}

		else {
			perror("read buff error!");
			exit(1);
		}
	}

  close(fd);

  //char *querString, *delim;
  //char mydata[1]="\0";
  //int len = 0;
  //int i = 0;

   //while(1) {
    //querString = getenv("QUERY_STRING");
    //delim = strchr(querString, '=');
    //strncpy(mydata,delim + 1,1);

    //printf("<p>querString : %s</p>", querString);
    //printf("<p>delim : %s</p>", delim);
    //printf("<p>mydata : %s</p>", mydata);

    // for(i=0; i<sizeof(delim); i++) {
    //    delim[i] = delim[i+1];
    // }
    // delim[sizeof(delim)] = "/0";

    /*
    if(delim) {
        len = strlen(mydata);
        *data = malloc(sizeof(mydata) *len);
        strcpy(*data, mydata);
    } else {
        len = 1;
        *data = malloc(sizeof(mydata) *len);
        **data = "\0";
    }
    */

    /*
      if(strcmp(mydata, "2") == 0) {
      // stop
         stop = true;
      }

      else if(strcmp(mydata, "3") == 0) {
      // clear
         timer = 0;
      }

      else if(strcmp(mydata, "1") == 0) {
      // start
         // perror("read buff error!");
         // exit(1);
         stop = false;
      }
    */
   //}
}


/*
void FndStopwatch()
{
  int i = 0;

  while (1) {
    for (i = 0; i < 6; i++) {
      FndDisplay(i, pos[i]);
      delay(1);
    }
    pos[0] += 1;
    if (pos[0] == 10) {
      pos[1] += 1;
      pos[0] = 0;
    }
    if (pos[1] == 10) {
      pos[2] += 1;
      pos[1] = 0;
    }
    if (pos[2] == 10) {
      pos[3] += 1;
      pos[2] = 0;
    }
    if (pos[3] == 10) {
      pos[4] += 1;
      pos[3] = 0;
    }
    if (pos[4] == 10) {
      pos[5] += 1;
      pos[4] = 0;
    }
    if (pos[5] == 10) {
      pos[5] = 0;
    }
  }
}
*/

int main() {
  pthread_t p_thread[2];
  int thr_id;
  int status;
  int a = 1;
  int b = 2;

  printf("Content-type:text/html\n\n");
  printf("<html>\n<head>\n<title>result</title>\n</head>\n<body>\n");

  int pid;
	pid = fork();
	if(pid > 0) {
		// Setup();
    Init();
	}

  // Init();

  else if(pid == 0) {

    // thread 시작
    thr_id = pthread_create(&p_thread[0], NULL, FndThread, (void *)&a);
    
    if(thr_id < 0) {
      perror("FndThread create error: ");
      exit(1);
    }
  
    thr_id = pthread_create(&p_thread[1], NULL, IPCThread, (void *)&b);

    if(thr_id <0) {
      perror("IPCThread create error: ");
      exit(1);
    }

    pthread_join(p_thread[0], (void **)&status);
    pthread_join(p_thread[1], (void **)&status);
    
    // printf("<p>main data = %s</p>", data);
    printf("</body>\n</html>");

  }

	else if(pid == -1) {
		perror("fork error: ");
		exit(1);
	}

  return 0;
}
