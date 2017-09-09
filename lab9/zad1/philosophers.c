#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "common.h"
#include <stdbool.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <unistd.h>
#include <sys/types.h>

#include <sys/time.h>

#define SHMNAME "/philosophers_semaphores"
pthread_t *threads_ids;
void **status_array;
sem_t waiter;
sem_t forks[5];
bool shouldContinue = true;

char* getTimeStamp(char timebuf[50]){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_t nowtime = tv.tv_sec;
	struct tm *nowtm = localtime(&nowtime);
	char tmbuf[50];
	strftime(tmbuf, 50, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(timebuf, 50, "%s.%06ld", tmbuf, tv.tv_usec);
	return timebuf;
}

void * philosopher_actions(void * index){
	char timebuf[50];
	while(shouldContinue){
		sleep(rand()%5); //thinking
		sem_wait(&waiter);
		sem_wait(&forks[*((int*)index)]);
		printf("%s thread %d took fork %d\n", getTimeStamp(timebuf), *((int*)index), *((int*) (index)));
		sem_wait(&forks[ (*((int*)index) + 1)%5 ]);
		printf("%s thread %d took fork %d\n", getTimeStamp(timebuf),*((int*)index), (*((int*)index) + 1) %5);
		sleep(rand()%3); //eating
		sem_post(&forks[*((int*)index)]);
		printf("%s thread %d put fork %d on the table\n", getTimeStamp(timebuf),*((int*)index), *((int*)index));
		sem_post(&forks[(*(int*)index + 1) %5]);
		printf("%s thread %d put fork %d on the table\n",getTimeStamp(timebuf), *((int*)index),  (*((int*)index) + 1) %5);
		sem_post(&waiter);
	}
	return (void *)19; //whatever
}

void clean(){
	CHECK(shm_unlink(SHMNAME) >= 0);
	CHECK(sem_destroy(&waiter) == 0);
	for (int i = 0; i < 5; i++){
		CHECK(sem_destroy(&forks[i]) == 0);
	}
}

void handle(int signo){
	shouldContinue = false;
}

void init(){
	CHECK((threads_ids = malloc(5 * sizeof(pthread_t))) != NULL);
	CHECK((status_array = malloc(5 * sizeof(void *))) != NULL);
	int shmid = shm_open(SHMNAME,O_CREAT | O_RDWR, 0777);
	//CHECK(ftruncate(shmid,6 * sizeof(sem_t)) == 0);
	CHECK(sem_init(&waiter, shmid,4) == 0);
	for (int i = 0; i< 5; i++){
		CHECK(sem_init(&forks[i],shmid,1) == 0);
	}
	atexit(clean);
	signal(SIGINT, handle);
	srand(time(NULL));
}

int main(){
	init();
	int index_array[5];
	for (int i = 0; i < 5 ; i++)
		index_array[i] = i;
	for(int i = 0; i<5; i++){
		CHECK(pthread_create(&threads_ids[i], NULL, &philosopher_actions, (void *)(&index_array[i])) == 0);
	}
	for (int i = 0; i<5; i++){
		CHECK(pthread_join(threads_ids[i], &status_array[i]) == 0);
		//printf("status of %lu: %d\n", threads_ids[i], ((int *) status_array)[i]);
	}
}
