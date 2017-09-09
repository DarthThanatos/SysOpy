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
#include <limits.h>

int shm_id;
sem_t *reader_sem, *writer_sem;
int *bufor;


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


void init(){
	CHECK( (shm_id = shm_open("common_space", O_RDWR, 0777)) >= 0);
	//CHECK(ftruncate(shm_id, 20) >= 0);
	printf("producer\n");
	bufor = (int *) mmap(0, 20, PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0);
	CHECK( (reader_sem = sem_open(READER_SEMAPHORE, 0) )>= 0);
	CHECK( (writer_sem = sem_open(WRITER_SEMAPHORE, 0) )>= 0);
}


void doJob(){
	int size = rand()%20;
	int *indecies = malloc(size * sizeof(int));
	for (int j = 0; j< size; j++){ 
		indecies[j] = rand()%20;
	}
	int x = rand()%50;
	for (int j = 0; j < size; j++){
		if(x != INT_MAX)x++;
		bufor[indecies[j]] = x;
		char timebuf[50];
		printf("(%d %s) wpisalem liczbe %d na pozycje %d pozostalo %d zadan\n", getpid(), getTimeStamp(timebuf), x, indecies[j], size - (j+1));
	}
}

int main(){
	init();
    srand(time(NULL));
    while(1){
		CHECK(sem_wait(writer_sem) >= 0);
		for (int i = 0; i< READERS_IN_LIBRARY; i++)
			CHECK(sem_wait(reader_sem) >= 0);
		doJob();
		for (int i =0 ; i< READERS_IN_LIBRARY ;i++)
			CHECK(sem_post(reader_sem) >= 0);
		CHECK(sem_post(writer_sem) >= 0);
    }
    return 0;
}

