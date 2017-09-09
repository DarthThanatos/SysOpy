#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include<stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include "common.h"
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

int shm_id;
sem_t *reader_sem;
int *bufor;
int x;
bool uOption = false;
int globalHowManyX = 0;

char* getTimeStamp(char timebuf[50]){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_t nowtime = tv.tv_sec;
	struct tm *nowtm = localtime(&nowtime);
	char tmbuf[50];
	strftime(tmbuf, 50, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(timebuf, 50, "%s.%06ld", tmbuf, tv.tv_usec);
	/*unsigned long long millisecondsSinceEpoch =
    		(unsigned long long)(tv.tv_sec) * 1000 +
   		 (unsigned long long)(tv.tv_usec) / 1000;*/
	return timebuf;
}


void init(int argc, char *argv[]){
	x = atoi(argv[1]); 
	if (argc == 3)
		uOption = true;
	CHECK( (shm_id = shm_open("common_space", O_RDWR, 0777)) >= 0);
	//CHECK(ftruncate(shm_id, 20) >= 0);
	printf("consumer\n");
	bufor = (int *) mmap(0, 20, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
	CHECK( (reader_sem = sem_open(READER_SEMAPHORE, 0) )>= 0);
}

void doJob(int i){
	if (bufor[i] == x){
		globalHowManyX ++;
	}
	char timebuf[50];
	if(! uOption)
		printf("( %d %s ) Znalazlem %d liczb o wartosci %d \n",getpid(), getTimeStamp(timebuf), globalHowManyX, x);

}

int main(int argc, char *argv[]){
	init(argc, argv);
	while(1){
		for (int i = 0; i<20; i++){
			CHECK(sem_wait(reader_sem) >= 0);
			doJob(i);
			CHECK(sem_post(reader_sem) >= 0);
		}
		if (uOption){
			char timebuf[50];
			printf("( %d %s ) Znalazlem %d liczb o wartosci %d \n",getpid(), getTimeStamp(timebuf), globalHowManyX, x);
		}
    }
    return 0;
}
