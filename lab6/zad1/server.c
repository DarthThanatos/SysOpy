#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#define CLIENT_MSG_TYPE 2
#define SERVER_MSG_TYPE 1
#define CLIENT_READY 3
#define CLIENT_NOT_READY 4
#define SERVER_GIVES_DISPATCH 5
#define CLIENTS_AMOUNT 3
int serverQueueId;
int serverFIFO_ID;
char *serverQueuePath;
volatile int queue_id;
int clients[CLIENTS_AMOUNT];
bool shouldContinue = true;

struct initReport{
	long mtype;
	int queue_id;
};

struct serverBack{
	long mtype;
	int num;
	int id;
	char mText[128];
};

void try(bool condition, char *msg){
	if(condition){
		printf("%s\n",msg);
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(3);
	}
}

void clean(){
	printf("SERVER: exiting . . .\n");
	try(msgctl(queue_id, IPC_RMID, NULL) < 0,"mgcntl failed");
	try(unlink(serverQueuePath) < 0,"unlinking failed");
}

void handler (int signo){
	shouldContinue = false;
}

void init(int argc, char *argv[]){
	try(argc!=3,"Wrong usage");
	serverQueueId = (int)argv[2][0];
	serverQueuePath = argv[1];
	for (int i = 0; i < CLIENTS_AMOUNT; i++){
		clients[i] = -1;
	}
	atexit(clean); //in case of normal exit
	signal(SIGINT, handler); //in case of ctrl + C
}

void generateFIFOFile(){
	try (close(open(serverQueuePath, O_WRONLY | O_CREAT, 0666)) < 0,
		"creating queue error");
	printf("Created fifo file\n");
}

void setServerFIFO_ID(){
	try((serverFIFO_ID = ftok(serverQueuePath,serverQueueId))<0,
	    "ftok failed");
	printf("created serverFIFO_ID : %d\n",serverFIFO_ID);
}

void setQueue_ID(){
	try((queue_id = msgget(serverFIFO_ID, IPC_CREAT | 0666)) < 0,
		"msget error");
	printf("Set queue id %d\n", queue_id);
}

void waitForClientsToRegister(){
	struct initReport clientReady;
	for (int i =0; i< CLIENTS_AMOUNT; i++){
		try(msgrcv(queue_id, &clientReady,  sizeof(clientReady.queue_id), CLIENT_MSG_TYPE, 0) < 0,"msgrcv error");
		printf("got message from %d\n", clientReady.queue_id);
		clients[i] = clientReady.queue_id;
		struct serverBack server_message;
		server_message.mtype = SERVER_MSG_TYPE;
		server_message.id = i;
		strcpy(server_message.mText,"hej");
		try(msgsnd(clientReady.queue_id, &server_message,sizeof(server_message.id) + sizeof(server_message.mText),0),"msg to client failed");
	}
}

void createServerQueue(){
	generateFIFOFile();
	setServerFIFO_ID();
	setQueue_ID();
}

struct secondPhase_ClientWantsNumber{
	long mType;
	int id;
};

struct secondPhase_ServerGivesNumber{
	long mType;
	int num;
};

struct secondPhase_ClientGivesNumber{
	long mType;
	int num;
	int id;
	int isPrime;
};

void giveDispatch(int clientId){
	int numRange = 100; //range of number selected by server to send to a client
	int numberToSend = rand()%numRange + 2; // +2 to eliminate 0 and 1
	struct secondPhase_ServerGivesNumber serverGives;
	serverGives.mType = SERVER_GIVES_DISPATCH;
	serverGives.num = numberToSend;
	try(msgsnd(clients[clientId], &serverGives, sizeof(serverGives.num), IPC_NOWAIT)<0,"sending did not work");
}

void receiveResult(struct secondPhase_ClientGivesNumber *clientGives){
	if(clientGives->isPrime){
		printf("Liczba pierwsza: %d klient: %d\n", clientGives->num,clientGives->id);
	}
}

void infiniteLoop(){
	srand(time(NULL));
	while (shouldContinue){
		struct secondPhase_ClientWantsNumber clientWants;
		//checks for requests
		try(msgrcv(queue_id, 
				&clientWants, 
				sizeof(clientWants.id),
				CLIENT_NOT_READY, 
				0)<0, 
			"phase two receive failed");
		int clientId = clientWants.id;
		giveDispatch(clientId);
		struct secondPhase_ClientGivesNumber clientGives;
		//check if results have come
		try(msgrcv(queue_id, &clientGives, 
				sizeof(clientGives.id) + sizeof(clientGives.isPrime) + sizeof(clientGives.num),
				CLIENT_READY , 0)<0,
			"phase two receive result failed\n");
		receiveResult(&clientGives);
	}
}

int main(int argc, char* argv[]){
	init(argc, argv);	
	createServerQueue();	
	waitForClientsToRegister();
	infiniteLoop();
}
