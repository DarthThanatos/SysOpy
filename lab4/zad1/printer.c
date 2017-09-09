#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

bool shouldExit = false;
bool eval = false;
bool up = true, change = false;
int textLength;
char *text;
int N;
int i = 1 ,r = -1;

void printText(){
	int j,k;
	r++;
	if (r == 2) change = true;
	if(change){
		if(up)i++; else i--;
		change = false;
		r = 0;
	}
	for(j = 0; j<i; j++){
		printf("%d. ",j);
		if(eval)
			for (k = textLength - 1; k>=0; k--)
				printf("%c",text[k]);
		else
			for (k = 0; k<textLength; k++)
				printf("%c",text[k]);
		printf("\n");
	}
	eval = eval == true ? false : true;
	if(i == N)
		up = false;
	if(i == 1)
		up = true;
}

static void sigAction_usr(int signo){
	switch(signo){
		case SIGTSTP:
			printText();
			break;
		default:
			printf("sigaction: Unknown signal received\n");
	}
}

void handleINT(){
	printf("received SIGINT - EXITING\n");
	shouldExit = true;
}

static void sig_usr(int signo){
	switch(signo){
		case SIGTSTP:
			printText();
			break;
		case SIGINT:
			handleINT();
			break;
	}
}

void checkParamaterers(int argc){
	if (argc != 3){
		printf("Wrong usage");
		exit(1);
	}
}

void setVars(char *argv[]){
	text = argv[1];
	N = atoi(argv[2]);
	textLength = strlen(text);
}

void activateSignals(){
	struct sigaction act;
	act.sa_handler = sigAction_usr;
	if (sigaction( SIGTSTP,&act,NULL) < 0){
		printf("can't catch SIGTSTP\n");
		exit(1);
	}
	if (signal( SIGINT,sig_usr) == SIG_ERR){
		printf("can't catch SIGINT\n");
		exit(1);
	}

}

void init(int argc, char *argv[]){
	checkParamaterers(argc);
	setVars(argv);
	activateSignals();
}

void infinitLoop(){
	while(!shouldExit){
		pause();
	}

}

int main(int argc, char *argv[]){
	init(argc,argv);
	infinitLoop();
}
