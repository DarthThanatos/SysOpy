#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include<stdlib.h>
#include <sys/shm.h>
#include <errno.h>
#include<unistd.h>
#include <sys/time.h>
#include <time.h>
#include "common.h"
#include <stdbool.h>

void closeSeamphore(int semid, int i){
    int nsops=1;
    struct sembuf *sopwait = (struct sembuf *) malloc(sizeof(struct sembuf));
    sopwait[0].sem_num = i;
    sopwait[0].sem_op = -1;
    sopwait[0].sem_flg = 0;
    CHECK( semop(semid, sopwait, nsops) >= 0);   
}

void riseSemaphore(int semid, int i){
    int nsops=1;
    struct sembuf *sops = (struct sembuf *) malloc( sizeof(struct sembuf));      
    sops[0].sem_num = i;
    sops[0].sem_op = 1;
    sops[0].sem_flg = 0;
    CHECK( semop(semid, sops, nsops) >= 0);
}

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

int main(){
    int i;
    int semid;
    key_t sem_key = 5678;
    int sem_flg = 0;
    int nsems = 0;
    int shmid;
    int *bufor;
    int shm_key = 9999;
    CHECK((semid = semget(sem_key, nsems, sem_flg) )>= 0);
    CHECK( (shmid = shmget(shm_key, 0, 0) )>= 0); 
    CHECK( (bufor = shmat(shmid, NULL, 0)) != (void*) -1); 
    srand(time(NULL));
    int x;
    while(1){
		i = 0;
		while(i < 20){
			x = rand() % 200 + 1;
			bool produced = false;
			closeSeamphore(semid,i);
			//printf("%d on %d\n",bufor[i], i);
			if(bufor[i] == 0){
				bufor[i] = x;
				//unsigned long long int timeStamp = getTimeStamp();
				int waitingForInc = 0;
				char timebuf[50];
				for (int j = 0; j<20; j++)
				  if (bufor[j]!=0)
					  waitingForInc ++;
					  //waitingForInc += semctl(semid,j,GETNCNT);
				printf("( %d %s )dodalem liczbe: %d na pozycje %d. Liczba zadan oczekujacych: %d\n",getpid(), getTimeStamp(timebuf), x, i, waitingForInc);
				produced = true;
			} 
			riseSemaphore(semid,i);
			if (produced) i++;
			//sleep(1);
		}
    }
}
