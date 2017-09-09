#include <stdio.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>

int serverQueueId;
int serverFIFO_ID;
char *serverQueuePath;
int msgqid;
int server_id;
bool shouldContinue = true;
int myClientId;

#define CLIENT_MSG_TYPE 2
#define SERVER_MSG_TYPE 1

#define CLIENT_READY 3
#define CLIENT_NOT_READY 4

#define SERVER_GIVES_DISPATCH 5

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
		exit(0);
	}
}


void clean(){
	printf("Exiting program\n");
	try(msgctl(msgqid,IPC_RMID,NULL), "closing error");
}

bool isPrime(int n){
	printf("Client %d checking if %d is Prime\n",myClientId,n);
	if (n < 2) return false;
	for (int i = 2; i*i<=n; i++){
		if (n % i == 0)
			return false;
	}
	return true;
}

void handler(int signo){
	shouldContinue = false;
}

void init(int argc, char *argv[]){
	try(argc!=3,"Wrong usage");
	serverQueueId = (int)argv[2][0];
	serverQueuePath = argv[1];	
	atexit(clean); // in case of normal exit
	signal(SIGINT, handler); //in case of ctrl + C
}

void setServerFIFO_ID(){
	try((serverFIFO_ID = ftok(serverQueuePath,serverQueueId))<0,
	    "ftok failed");
	printf("set ServerFIFO_ID %d\n",serverFIFO_ID);
}

void setServer_key(){
	try((server_id = msgget(serverFIFO_ID,0))<0,"msgget server error");
	printf("setserverKey\n");
}

void createClientQueue(){
	msgqid = msgget(IPC_PRIVATE, 0666|IPC_CREAT|IPC_EXCL);	
	printf("Created own queue, id: %d\n", msgqid);
}

void reportReadiness(){
	struct initReport clientReady;
	clientReady.queue_id = msgqid;
	clientReady.mtype = CLIENT_MSG_TYPE;
	try((msgsnd(server_id, &clientReady, sizeof(clientReady.queue_id),0)<0),"Could not send msg");
	printf("reported\n");
}

void waitForResponse(){
	struct serverBack server_message;
	try(msgrcv(msgqid,&server_message,
			sizeof(server_message.id) + sizeof(server_message.mText),SERVER_MSG_TYPE,0) < 0, 
		"server response failed");
	printf("received text: %s and id: %d\n",server_message.mText, server_message.id);
	myClientId = server_message.id;
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

void infiniteLoop(){
	for(int i = 0; i < 10 ;i++){
		struct secondPhase_ClientWantsNumber clientWants;
		clientWants.id = myClientId;
		clientWants.mType = CLIENT_NOT_READY;
		//try to gain a dispatch
		try(msgsnd(server_id, &clientWants,sizeof(clientWants.id),IPC_NOWAIT)<0, "second phase- did not send message");
		struct secondPhase_ServerGivesNumber serverGives;
		printf("Send request\n");
		//wait for dispatch
		printf("Waiting for dispatch\n");
		try(msgrcv(msgqid, &serverGives,sizeof(serverGives.num), SERVER_GIVES_DISPATCH,0)<0,"second phase did not receive msg");
		struct secondPhase_ClientGivesNumber clientGives;
		clientGives.isPrime = isPrime(serverGives.num);
		clientGives.num = serverGives.num;
		clientGives.id = myClientId;
		clientGives.mType = CLIENT_READY;
		//send response
		try(msgsnd(server_id, &clientGives,
				sizeof(clientGives.id) + sizeof(clientGives.isPrime) + sizeof(clientGives.num),0)<0, 
			"second phase - did not return result");
	}
}

int main(int argc, char* argv[]){
	init(argc, argv);	
	createClientQueue();	
	setServerFIFO_ID();
	setServer_key();
	reportReadiness();
	waitForResponse();
	infiniteLoop();
}
