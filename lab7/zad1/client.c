#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include<stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include "common.h"


void closeSemaphore(int semid, int i){
    int err,nsops=1;
    struct sembuf *sopwait = (struct sembuf *) malloc(sizeof(struct sembuf));
    sopwait[0].sem_num = i;
    sopwait[0].sem_op = -1;
    sopwait[0].sem_flg = 0;
    err = semop(semid, sopwait, nsops);
    if(err < 0)
        perror(" unable to do the sop \n");
}

void riseSemaphore(int semid, int i){
    int err,nsops=1;
    struct sembuf *sops = (struct sembuf *) malloc(sizeof(struct sembuf));      
    sops[0].sem_num = i;
    sops[0].sem_op = 1;
    sops[0].sem_flg = 0;
    err = semop(semid, sops, nsops);
    if(err < 0)
        printf(" unable to do the sop \n");
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


int main()
{
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
	while(1){
		i = 0;
		while(i < 20){
			bool consumed = false;
			closeSemaphore(semid,i);
			//printf("%d on %d\n",bufor[i], i);
			if(bufor[i] != 0){
				//unsigned long long int timeStamp = getTimeStamp();
				int product = bufor[i];
				bufor[i] = 0;
				int waitingForInc = 0;
				for (int j = 0; j<20; j++)
					if(bufor[j] != 0)
						waitingForInc ++;
				  //waitingForInc += semctl(semid,j,GETNCNT);
				char timebuf[50];
				if (bufor[i]%2 ==0)
					printf("( %d %s )otrzymalem liczbe: %d na pozycji %d - parzysta. Liczba zadan oczekujacych: %d\n",getpid(), getTimeStamp(timebuf), product, i, waitingForInc );
				else
					printf("( %d %s )otrzymalem liczbe: %d na pozycji %d - nieparzysta. Liczba zadan oczekujacych: %d\n",getpid(), getTimeStamp(timebuf), product, i, waitingForInc );

				consumed = true;
			} 
			riseSemaphore(semid,i);
			if (consumed) i++;
			//sleep(1);
		}
    }
    return 0;
}
