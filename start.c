#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <pthread.h>
#include <wiringPi.h>

#define FIFO_FILE   "/tmp/fifo"
#define BUFF_SIZE   1024

const int FndSelectPin[6] = {4, 17, 18, 27, 22, 23};
const int FndPin[8] = {6, 12, 13, 16, 19, 20, 26, 21};
const int FndFont[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};
int pos[6] = {0, 0, 0, 0, 0, 0};
int timer = 0;
bool stop = false;
pthread_mutex_t mutex;

void Setup() {
    int i;

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

void* FndThread(void* arg) {
    while (1) {
        int i;
        int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        pthread_mutex_lock(&mutex);  // 뮤텍스 잠금

        for (i = 0; i < 6; i++) {
            FndDisplay(i, data[(timer / (int)pow(10, i)) % 10]);
            delay(1);
            if ((i + timer) % 2 == 0)
                delay(1);
        }

        pthread_mutex_unlock(&mutex);  // 뮤텍스 해제

        delay(1);

        if (stop == true) { continue; }

        pthread_mutex_lock(&mutex);  // 뮤텍스 잠금
        timer++;
        if (timer >= 1000000) {
            timer = 0;
        }
        pthread_mutex_unlock(&mutex);  // 뮤텍스 해제
    }
}

void IPCThread() {
    int fd;
    char buff[BUFF_SIZE];

    while (1) {
        fd = open(FIFO_FILE, O_RDONLY);

        memset(buff, 0, BUFF_SIZE);
        read(fd, buff, BUFF_SIZE);

        if (strcmp(buff, "") != 0) {
            if (strcmp(buff, "2") == 0) {
                pthread_mutex_lock(&mutex);  // 뮤텍스 잠금
                stop = true;
                pthread_mutex_unlock(&mutex);  // 뮤텍스 해제
                perror("stop error! : ");
            }
            else if (strcmp(buff, "3") == 0) {
                pthread_mutex_lock(&mutex);  // 뮤텍스 잠금
                timer = 0;
                pthread_mutex_unlock(&mutex);  // 뮤텍스 해제
                perror("clear error! : ");
            }
            else if (strcmp(buff, "1") == 0) {
                pthread_mutex_lock(&mutex);  // 뮤텍스 잠금
                stop = false;
                pthread_mutex_unlock(&mutex);  // 뮤텍스 해제
                perror("press start");
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


bool isProcessRunning(pid_t processId) {
    char procPath[256];
    snprintf(procPath, sizeof(procPath), "/proc/%d", processId);

    FILE* procFile = fopen(procPath, "r");
    if (procFile != NULL) {
        fclose(procFile);
        return true;  // 프로세스가 존재함
    }
    return false;  // 프로세스가 존재하지 않음
}


int main() {
    setbuf(stdout, NULL);

    printf("Content-type:text/html\n\n");
    printf("<html>\n<head>\n<title>STOP WATCH PROGRAM</title>\n</head>\n");
    printf("<body>\n<p>Stop Watch</p>\n</br>\n");
    printf("<form method=get action='start.cgi'>\n");
   printf("<input type='submit' name='button' value='Start'>\n</form>\n");
   printf("<form method=get action='stop.cgi'>\n");
   printf("<input type='submit' name='button' value='Stop'>\n</form>\n");
   printf("<form method=get action='clear.cgi'>\n");
   printf("<input type='submit' name='button' value='Clear'>\n</form>\n");
   printf("</body>\n</html>");

    pthread_t fndThread;
    int result;

    // bool is_alive = false;
    char* content = "\0";
    
    FILE * file;
    char buff_pid[BUFF_SIZE];
    int pid;
    int fd;
	  char *st = "1";

    file = fopen("process_id.txt", "r"); // 'r' : 열기만 생성X
    if (file == NULL)
    {
      file = fopen("process_id.txt", "w+"); // 파일 생성
      fclose(file);
      perror("process_id file don't exist.");
    }
    else // file 열림
    {
      fgets( buff_pid, sizeof(buff_pid), file);
      pid = atoi(buff_pid);
      fclose(file);
    }


    if(isProcessRunning(pid)) // process true
    {
      fd = open(FIFO_FILE, O_WRONLY);
      write(fd, st, strlen(st));
      close(fd);
    }
    else // process 없음
    {
      if (pthread_mutex_init(&mutex, NULL) != 0) {
          printf("Mutex initialization failed.\n");
          return 1;
      }

      Setup();

      result = pthread_create(&fndThread, NULL, FndThread, NULL);
      if (result != 0) {
          printf("Failed to create FndThread.\n");
          return 1;
      }
      // pid 저장
      // pid = getpid();
      file = fopen("process_id.txt", "w+");
      fprintf(file, "%d", getpid());
      fclose(file);

      IPCThread();

      pthread_join(fndThread, NULL);
      pthread_mutex_destroy(&mutex);

    }
    
    printf("</body>\n</html>");

    return 0;
}
