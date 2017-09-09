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

void inc_signals_received(int signo){
	received++;
	printf("sender - received SIGRTMIN+1 %d\n", received);
}

void stop_working(int signo){
	printf("sender - received %d signals, should receive %d"
			"\nending program\n", received, N);
	shouldContinue = false;
}


void shootSignals(){
	int i;
    union sigval sig_value;
	for (i = 0; i < N; i++){
	    if (sigqueue(pid, SIGRTMIN + 1, sig_value) < 0) {
	        perror("sigqueue");
	        exit(EXIT_FAILURE);
	    }
	}
    if (sigqueue(pid, SIGRTMIN + 2, sig_value) < 0) {
        perror("sigqueue");
        exit(EXIT_FAILURE);
    }
}

void activateSignals(){
	if (signal( SIGRTMIN+1,inc_signals_received) == SIG_ERR){
		printf("cant catch SIGRTMIN+1\n");
		exit(1);
	}
	if (signal( SIGRTMIN+2,stop_working) == SIG_ERR){
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
