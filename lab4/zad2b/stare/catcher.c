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

void shootSignalsBack(){
	int i;
	for (i = 0; i < received; i++){
		//printf("catcher - SHOT\n");
		kill(pid,SIGUSR1);
	}
	//printf("catcher - sent sigusr2\n");
	kill(pid, SIGUSR2);

}

void handleUSR1(){
	printf("catcher - received USR1\n");
	setSenderPid();
	received++;
	kill(pid,SIGUSR1);
}

void handleUSR2(){
	//shootSignalsBack();
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
	if (signal( SIGUSR1,sig_usr) == SIG_ERR){
		printf("cant catch SIGUSR1\n");
		exit(1);
	}
	if (signal( SIGUSR2,sig_usr) == SIG_ERR){
		printf("cant catch SIGUSR2\n");
		exit(1);
	}
}

void infiniteLoop(){
	while(shouldContinue){
		pause();
	}
}

int main(){
	activateSignals();
	infiniteLoop();
}
