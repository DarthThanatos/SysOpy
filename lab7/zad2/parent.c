#include "common.h"
#include <stdio.h>
#include <stdbool.h> 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int consumers_amount, producers_amount;
char *writerProgram[] = {"./writer", 0};
char *readerUProgram[] = {"./reader", "0", "-u",0};
char *readerProgram[] = {"./reader","0", 0};
int shmid;
int *bufor; 
bool shouldContinue = true;
bool uOption = false;

void getNumber(char number[50]){
	srand(time(NULL));
	int x = rand()%50;
	memset(number,0,50);
	sprintf(number, "%d", x);
}

void handler(int signo){
	shouldContinue = false;
}

void clean(){
	CHECK(shm_unlink(SPACE_NAME) >= 0);
	CHECK(sem_unlink(WRITER_SEMAPHORE) >=0);
	CHECK(sem_unlink(READER_SEMAPHORE) >=0);
}

void try(bool condition, char *msg){
	if(condition){
		printf("%s\n",msg);
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(3);
	}
}

void init(int argc, char* argv[]){
	signal(SIGINT,handler);
	atexit(clean);
	try(argc != 4 && argc != 3 , "usage: ./parent producers_amount clients_amount [-u]");
	producers_amount = atoi(argv[1]);
	consumers_amount = atoi(argv[2]);
	if (argc == 4 && strcmp(argv[3],"-u") == 0) {
		uOption = true; 
		printf("uoption \n");
	}
    shmid = shm_open(SPACE_NAME, O_CREAT| O_RDWR, 0777); 
	CHECK(ftruncate(shmid, 20) >= 0);
	bufor = (int *)mmap(0, 20,PROT_WRITE | PROT_READ,MAP_SHARED,shmid,0);
	printf("%x\n",bufor);
	for (int i = 0; i < 20; i++){
		bufor[i] = 0;
	}
	sem_t *writer_sem, *reader_sem;
	CHECK((writer_sem = sem_open(WRITER_SEMAPHORE, O_CREAT,0777,1)) > 0);
	CHECK((reader_sem = sem_open(READER_SEMAPHORE, O_CREAT,0777,READERS_IN_LIBRARY)) > 0);
	CHECK(sem_close(writer_sem) >= 0); 
	CHECK(sem_close(reader_sem) >= 0); 
}

void activateReaders(){
	srand(time(NULL));
	for (int i = 0; i < producers_amount; i++){
		char number[50];
		getNumber(number);
		readerUProgram[1] = number;
		readerProgram[1] = number;
		switch(fork()){
			case -1:
				printf("fork failed\n");
				exit(0);
			case 0:
				if (uOption)
					CHECK(execvp(readerUProgram[0], readerUProgram) != -1);
				else
					CHECK(-1 != execvp(readerProgram[0], readerProgram));
			default: //parent continues creating children
				printf("reader activation\n");
				break;
		}
	}
}

void activateWriters(){
	for (int i = 0; i < consumers_amount; i++){
		switch(fork()){
			case -1:
				printf("fork failed\n");
				exit(0);
			case 0:
				CHECK(execvp(writerProgram[0], writerProgram) != -1);
			default: //parent continues creating children
				printf("writers activation\n");
				break;
		}
	}
}

int main(int argc, char * argv[]){
	init(argc, argv);
	activateWriters();
	activateReaders();
	while(shouldContinue);
}
