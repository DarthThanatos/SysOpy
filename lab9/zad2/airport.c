#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "common.h"
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define N 15
#define K 6
#define AIRPLANES_AMOUNT 20

int onAirport = 0;
pthread_t *threads_ids;
void **status_array;
bool shouldContinue = true;

pthread_mutex_t mutex =  PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t landing = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t starting = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t tryingToLand  = PTHREAD_COND_INITIALIZER;
pthread_cond_t tryingToStart  = PTHREAD_COND_INITIALIZER;

bool freeLandingArea = true;
int cond_waiting_to_start = 0;
int cond_waiting_to_land = 0;

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

void clean(){
	pthread_cond_destroy(&tryingToStart);
	pthread_cond_destroy(&tryingToLand);
}

void handle(int signo){
	shouldContinue = false;
}

void init(){
	CHECK((threads_ids = malloc(AIRPLANES_AMOUNT * sizeof(pthread_t))) != NULL);
	CHECK((status_array = malloc(AIRPLANES_AMOUNT * sizeof(void *))) != NULL);
	atexit(clean);
	signal(SIGINT, handle);
	srand(time(NULL));
}

void takeOff(int index){
	onAirport--;
	
	pthread_mutex_lock(&starting);
	cond_waiting_to_start--;
	pthread_mutex_unlock(&starting);
	
	pthread_cond_signal(&tryingToStart);
	char timebuf[50];
	printf("%s thread %d started on airport %d\n",getTimeStamp(timebuf),  index, onAirport);
}

void tryToTakeOff(int index){

	pthread_mutex_lock(&starting);
	cond_waiting_to_start++;
	pthread_mutex_unlock(&starting);
	
	pthread_mutex_lock(&mutex);
	pthread_mutex_lock(&landing);
	while(onAirport < K && cond_waiting_to_land > 0){
		pthread_mutex_unlock(&landing);
		pthread_cond_wait(&tryingToLand,&mutex);
		printf("got signal Start: %d\n", index);
		pthread_mutex_lock(&landing);
	}
	pthread_mutex_unlock(&landing);
	takeOff(index);
	pthread_mutex_unlock(&mutex);
}

void land(int index){
	onAirport++;

	pthread_mutex_lock(&landing);
	cond_waiting_to_land--;
	pthread_mutex_unlock(&landing);

	char timebuf[50];
	printf("%s thread %d landed, on airport: %d\n",getTimeStamp(timebuf),  index, onAirport);
	pthread_cond_signal(&tryingToStart);
}

void tryToLand(int index){
	pthread_mutex_lock(&landing);
	cond_waiting_to_land++;
	pthread_mutex_unlock(&landing);

	pthread_mutex_lock(&mutex);
	pthread_mutex_lock(&starting);
	while((cond_waiting_to_start>0 && onAirport >= K) || onAirport == N){
		pthread_mutex_unlock(&starting);
		pthread_cond_wait(&tryingToStart,&mutex);
		pthread_mutex_lock(&starting);
	}
	pthread_mutex_unlock(&starting);
	land(index);
	pthread_mutex_unlock(&mutex);
}

void * airplane_actions(void * index){
	int _index = *((int*)index);
	while(shouldContinue){
		sleep(1); //flying rand()%3
		tryToLand(_index); 
		sleep(1); //resting rand()%3
		tryToTakeOff(_index);  
	}
	return (void *)19; //whatever
}


int main(){
	init();
	int index_array[AIRPLANES_AMOUNT];
	for (int i = 0; i < AIRPLANES_AMOUNT ; i++)
		index_array[i] = i;
	for(int i = 0; i<AIRPLANES_AMOUNT; i++){
		CHECK(pthread_create(&threads_ids[i], NULL, &airplane_actions, (void *)(&index_array[i])) == 0);
	}
	/*for (int i = 0; i<AIRPLANES_AMOUNT; i++){
		printf("checking status %d %d\n", i, AIRPLANES_AMOUNT);
		CHECK(pthread_join(threads_ids[i], &status_array[i]) == 0);
	}*/
	while(shouldContinue);
}
