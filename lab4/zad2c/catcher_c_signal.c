#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

bool shouldContinue = true;
int received = 0;
pid_t pid;

void setSenderPid(){
	FILE *sender = popen("pidof sender_c_signal", "r");
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
		kill(pid,SIGRTMIN+1);
	}
	//printf("catcher - sent sigusr2\n");
	kill(pid, SIGRTMIN+2);

}

void handleUSR1(){
	received++;
	printf("catcher - received SIGRTMIN+1 %d\n", received);
}

void handleUSR2(){
	setSenderPid();
	shootSignalsBack();
	shouldContinue = false;
}


void activateSignals(){
	if (signal( SIGRTMIN+1,handleUSR1) == SIG_ERR){
		printf("cant catch SIGRTMIN+1\n");
		exit(1);
	}
	if (signal( SIGRTMIN+2,handleUSR2) == SIG_ERR){
		printf("cant catch SIGRTMIN+2\n");
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
