#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

bool shouldContinue = true;
int received = 0, N;
pid_t pid;
bool catcherReported = false;

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
			//printf("sender received\n");
			received++;
			catcherReported = true;
			break;
		case SIGUSR2:
			printf("sender - received %d signals, should receive %d"
					"\nending program\n", received, N);
			shouldContinue = false;
			break;
		default:
			printf("sender got unknown signal\n");
	}
}

void shootSignals(){
	int i;
	sigset_t mask, oldmask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGUSR1);
	sigprocmask(SIG_BLOCK,&mask,&oldmask);
	for (i = 0; i < N; i++){
		kill(pid,SIGUSR1);
		sigsuspend(&oldmask);
	}
	if (signal( SIGUSR2,sig_usr) == SIG_ERR){
		printf("cant catch SIGUSR2\n");
		exit(1);
	}
	kill(pid, SIGUSR2);
}

void activateSignals(){
	struct sigaction act;
	act.sa_handler = sig_usr;
	if(sigaction(SIGUSR1, &act, NULL) == -1)
                printf("SIGUSR1 ERROR");
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
        sigset_t set,oset;
        sigemptyset(&set);
        sigprocmask(SIG_BLOCK,&set,&oset);
        while(shouldContinue){
                sigsuspend(&oset);
        }
}

int main(int argc, char *argv[]){
	init(argc,argv);
	shootSignals();
	infiniteLoop();
}
