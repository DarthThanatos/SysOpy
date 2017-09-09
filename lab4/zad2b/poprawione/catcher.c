#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

bool shouldContinue = true;
int received = 0;
pid_t pid;
bool senderReported = false;

void setSenderPid(){
	FILE *sender = popen("pidof sender", "r");
	if (sender == NULL){
		printf("could not find the specified file\n");
		exit(1);
	}
	if(fscanf(sender,"%d",&pid) == EOF){
		printf("sender is not on\n");
		exit(1);
	}
	fclose(sender);
}

void handleUSR1(){
	received++;
	if(received == 1)
	    setSenderPid();
	kill(pid,SIGUSR1);
}

void handleUSR2(){
	kill(pid,SIGUSR2);
	printf("catcher - received SIGUSR2, ending program\n");
	shouldContinue = false;
}

static void sig_usr(int sig){
	switch(sig){
		case SIGUSR1:
			handleUSR1();
			break;
		case SIGUSR2:
			handleUSR2();
			break;
	}
}

void activateSignals(){
	struct sigaction act;
	act.sa_handler = sig_usr;
	if(sigaction(SIGUSR1, &act, NULL) == -1)
		printf("SIGUSR1 ERROR");
	if(sigaction(SIGUSR2, &act, NULL) == -1)
		printf("SIGUSR2 ERROR");
}

void infiniteLoop(){
	sigset_t set,oset;
	sigemptyset(&set);
	sigprocmask(SIG_BLOCK,&set,&oset);
	while(shouldContinue){
		sigsuspend(&oset);
	}
}

int main(){
	activateSignals();
	infiniteLoop();
}
