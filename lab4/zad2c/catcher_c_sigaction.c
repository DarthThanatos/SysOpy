#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

bool shouldContinue = true;
int received = 0;
pid_t pid;

void setSenderPid(){
	FILE *sender = popen("pidof sender_c_sigaction", "r");
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

void handleUSR1(int sig_number,siginfo_t *info,void * arg __attribute__ ((unused))){
	received++;
	//printf("catcher - received SIGRTMIN+1 %d\n", received);
}

void handleUSR2(int sig_number,siginfo_t *info,void * arg __attribute__ ((unused))){
	setSenderPid();
	shootSignalsBack();
	shouldContinue = false;
}


void activateSignals(){
	struct sigaction action1,action2;
	sigfillset(&action1.sa_mask);
	sigfillset(&action2.sa_mask); //no other signals may interrupt this one
	action1.sa_sigaction = handleUSR1;
	action2.sa_sigaction = handleUSR2;
	if (sigaction(SIGRTMIN+1, &action1,NULL) < 0){
		printf("cant catch SIGRTMIN+1\n");
		exit(1);
	}
	if (sigaction(SIGRTMIN+2, &action2,NULL) < 0){
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
