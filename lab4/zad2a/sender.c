#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

bool shouldContinue = true;
int received = 0, N;
pid_t pid;

void setCatcherPid(){
	FILE *catcher = popen("pidof catcher", "r");
	if (catcher == NULL){
		printf("could not find the specified file\n");
		exit(1);
	}
	if(fscanf(catcher,"%d",&pid) == EOF){
		printf("catcher is not on\n");
		exit(1);
	}
	fclose(catcher);
}

static void sig_usr(int sig){
	switch(sig){
		case SIGUSR1:
			printf("sender - received USR1\n");
			received++;
			break;
		case SIGUSR2:
			printf("sender - received %d signals, should receive %d"
					"\nending program\n", received, N);
			shouldContinue = false;
			break;
	}
}

void shootSignals(){
	int i;
	for (i = 0; i < N; i++){
		printf("sender - SHOT\n");
		kill(pid,SIGUSR1);
	}
	//printf("sender - sent sigusr2\n");
	kill(pid, SIGUSR2);
}

void activateSignals(){
	if (signal( SIGUSR1,sig_usr) == SIG_ERR){
		printf("cant catch SIGUSR1\n");
		exit(1);
	}
	if (signal( SIGUSR2,sig_usr) == SIG_ERR){
		printf("cant catch SIGUSR2\n");
		exit(1);
	}
}

void setN(char *argv[]){
	N = atoi(argv[1]);
}

void checkParameters( int argc){
	if (argc != 2){
		printf("Wrong usage\n");
		exit(1);
	}
}

void init(int argc, char* argv[]){
	checkParameters(argc);
	setN(argv);
	setCatcherPid();
	activateSignals();
}

void infiniteLoop(){
	while(shouldContinue){
		pause();
	}
}

int main(int argc, char *argv[]){
	init(argc,argv);
	shootSignals();
	infiniteLoop();
}
