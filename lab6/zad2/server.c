#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "common.h"

char *serverPath;
int clients[CLIENTS_AMOUNT];
char clientPaths[CLIENTS_AMOUNT][21];
bool shouldContinue = true;
mqd_t serverDescriptor;
struct mq_attr queueAtributes;
int registered = 0;

void try(bool condition, char *msg){
	if(condition){
		printf("%s\n",msg);
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(3);
	}
}

void clean(){
	printf("SERVER: exiting . . .\n");
	CHECK((mqd_t)-1 != mq_unlink(serverPath));
	for (int i =0 ; i< CLIENTS_AMOUNT; i++)
		CHECK((mqd_t)-1 !=mq_close(clients[i]));
	CHECK((mqd_t)-1 != mq_close(serverDescriptor));
}

void handler (int signo){
	shouldContinue = false;
}

void giveDispatch(int clientId){
	int numRange = 100; //range of number selected by server to send to a client
	int numberToSend = rand()%numRange + 2; // +2 to eliminate 0 and 1
	struct secondPhase_ServerGivesNumber serverGives;
	serverGives.mType = SERVER_GIVES_DISPATCH;
	serverGives.num = numberToSend;
	mqd_t clientDescriptor = mq_open(clientPaths[clientId], O_WRONLY);
	//printf("got msg from id: %d\n", clientWants->id);
	CHECK((mq_send(clientDescriptor,
				  (char *) &serverGives, 
				  sizeof(struct secondPhase_ServerGivesNumber), 0)) >= 0);
	//printf("sent dispatch to %d\n", clientWants->id);
}

void receiveResult(struct secondPhase_ClientGivesNumber *clientGives){
	if(clientGives->isPrime){
		printf("Liczba pierwsza: %d klient: %d\n", clientGives->num,clientGives->id);
	}
}


void init(int argc, char *argv[]){
	try(argc!=2,"Wrong usage: ./server SERVER_PATH");
	serverPath = argv[1];
	printf("serverPath: %s\n", serverPath);
	for (int i = 0; i < CLIENTS_AMOUNT; i++){
		clients[i] = -1;
	}
	atexit(clean); //in case of normal exit
	signal(SIGINT, handler); //in case of ctrl + C
}

void createServerQueue(){
	serverDescriptor = mq_open(serverPath, O_CREAT | O_RDONLY, 0644,NULL );
	CHECK((mqd_t)-1 != serverDescriptor);
}

void waitForClientsToRegister(){
	struct mq_attr attr;
	attr.mq_flags = O_NONBLOCK;
	mq_setattr(serverDescriptor, &attr,NULL);
	int i = 0;
	while(i<CLIENTS_AMOUNT){
		struct initReport *clientReady;
		char check[50]; 
		mq_receive(serverDescriptor, check, 8192 , NULL);
		switch(check[0]){
			case 'a':
				clientReady = (struct initReport *) check;
				printf("got message from %s\n", clientReady->queue_id);
				clients[i] = mq_open(clientReady->queue_id, O_WRONLY);
				CHECK((mqd_t) -1 != clients[i]);
				strcpy(clientPaths[i],clientReady->queue_id);
				i++;
				break;
		}
		memset(check,0,50);
	}
	for (i =0; i < CLIENTS_AMOUNT; i++){ 
		//sends ids only when the amount of clients defined in CLIENTS_AMOUNT have reported
		struct serverBack server_message;
		server_message.mtype = SERVER_MSG_TYPE;
		server_message.id = i;
		strcpy(server_message.mText,"hej");
		CHECK(mq_send(clients[i], (char*)&server_message, sizeof(struct serverBack), 0) >= 0);
	}
}

void infiniteLoop(){
	srand(time(NULL));
	struct secondPhase_ClientWantsNumber * clientWants;
	struct secondPhase_ClientGivesNumber * clientGives;
	struct clientExits *bye;
	char check[50]; 
	while (shouldContinue){
		mq_receive(serverDescriptor, check, 8192 , NULL);
		switch(check[0]){
			case 'b':
				clientWants = (struct secondPhase_ClientWantsNumber *)check;
				giveDispatch(clientWants->id);
				break;
			case 'c':
				clientGives = (struct secondPhase_ClientGivesNumber *)check;
				receiveResult(clientGives);
				break;
			case 'd':
				bye = (struct clientExits *)check;
				printf("Client %d has terminated\n", bye->id);
				break;
		}
		memset(check,0,50);
	}
}

int main(int argc, char* argv[]){
	init(argc, argv);
	createServerQueue();
	waitForClientsToRegister();
	infiniteLoop();
}
