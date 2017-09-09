#include "common.h"
#include <stdio.h>
#include <stdbool.h> 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

int consumers_amount, producers_amount;
char *producerProgram[] = {"./server", 0};
char *consumerProgram[] = {"./client", 0};
key_t shm_key = 9999, sem_key = 5678; 
int shmid, semid;
int *bufor; 
bool shouldContinue = true;


void handler(int signo){
	shouldContinue = false;
}

void clean(){
	CHECK(shmctl(shmid, IPC_RMID, 0) >= 0);
	semid = semget(sem_key,0,0666);
	CHECK(semctl(semid,0,IPC_RMID) >=0);
}

void try(bool condition, char *msg){
	if(condition){
		printf("%s\n",msg);
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(3);
	}
}

void init(int argc, char* argv[]){
	try(argc != 3, "usage: ./parent producers_amount clients_amount");
	producers_amount = atoi(argv[1]);
	consumers_amount = atoi(argv[2]);
    shmid = shmget(shm_key, 30, IPC_CREAT|0666); 
    semid = semget(sem_key, 20, IPC_CREAT|0666);
	for (int i = 0; i<20; i++)
		semctl(semid, i, SETVAL, 1) ;  
	bufor = shmat(shmid, 0,0);
	for (int i = 0; i < 20; i++)
		bufor[i] = 0;
	shmdt(bufor);
	signal(SIGINT,handler);
	atexit(clean);
}

void activateProducers(){
	for (int i = 0; i < producers_amount; i++){
		switch(fork()){
			case -1:
				printf("fork failed\n");
				exit(0);
			case 0:
				CHECK(-1 != execvp(producerProgram[0], producerProgram));
			default: //parent continues creating children
				printf("producers activation\n");
				break;
		}
	}
}

void activateConsumers(){
	for (int i = 0; i < consumers_amount; i++){
		switch(fork()){
			case -1:
				printf("fork failed\n");
				exit(0);
			case 0:
				CHECK(execvp(consumerProgram[0], consumerProgram) != -1);
			default: //parent continues creating children
				printf("consumers activation\n");
				break;
		}
	}
}

int main(int argc, char * argv[]){
	init(argc, argv);
	activateProducers();
	activateConsumers();
	while(shouldContinue);
}
