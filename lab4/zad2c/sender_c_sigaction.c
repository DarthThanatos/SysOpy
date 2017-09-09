#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

bool shouldContinue = true;
int received = 0, N;
pid_t pid;

void setCatcherPid(){
	FILE *catcher = popen("pidof catcher_c_sigaction", "r");
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

void inc_signals_received(int sig_number,siginfo_t *info,void * arg __attribute__ ((unused))){
	received++;
	//printf("sender - received SIGRTMIN+1 %d\n", received);
}

void stop_working(int sig_number,siginfo_t *info,void * arg __attribute__ ((unused))){
	printf("sender - received %d signals, should receive %d"
			"\nending program\n", received, N);
	shouldContinue = false;
}


void shootSignals(){
	int i;
	for (i = 0; i < N; i++){
		//printf("sender - SHOT\n");
		kill(pid,SIGRTMIN+1);
	}
	//printf("sender - sent sigusr2\n");
	kill(pid, SIGRTMIN+2);
}

void activateSignals(){
	struct sigaction action1,action2;
	sigfillset(&action1.sa_mask);
	sigfillset(&action2.sa_mask); //no other signals may interrupt this one
	action1.sa_sigaction = inc_signals_received;
	action2.sa_sigaction = stop_working;
	if (sigaction(SIGRTMIN+1, &action1,NULL) < 0){
		printf("cant catch SIGRTMIN+1\n");
		exit(1);
	}
	if (sigaction(SIGRTMIN+2, &action2,NULL) < 0){
		printf("cant catch SIGRTMIN+2\n");
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
