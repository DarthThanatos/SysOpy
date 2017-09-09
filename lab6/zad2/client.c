#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <string.h>
#include "common.h"

char *serverQueueName;
pid_t serverPid;
bool shouldContinue = true;
int myClientId;

mqd_t myDescriptor;
mqd_t serverDescriptor;
char *serverPath;
char myPath[21];
char myAliasPath[21];
struct mq_attr queueAtributes;

void try(bool condition, char *msg){
	if(condition){
		printf("%s\n",msg);
		exit(0);
	}
}

void setUniqueQueueName(char Path[21],bool alias){
   char s_pid[20];
   memset(Path,0,21);
   pid_t pid = getpid();
   strcat(Path,"/");
   sprintf(s_pid,"%d", pid);
   strcat(Path,s_pid);
   if (alias) strcat(Path,"a");
}


void clean(){
	printf("Exiting program\n");
	CHECK((mqd_t) -1 != mq_close(myDescriptor));
	CHECK((mqd_t) -1 != mq_close(serverDescriptor));
	CHECK((mqd_t) -1 != mq_unlink(myPath));
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

void setServerPid(){
	FILE *fp = popen("pidof ./server", "r");
	fscanf(fp,"%d",&serverPid);
}

void init(int argc, char *argv[]){
	try(argc!=2,"Wrong usage");
	serverPath = argv[1];
	setServerPid();
	setUniqueQueueName(myPath, false);
	serverDescriptor = mq_open(serverPath, O_WRONLY);
	atexit(clean); // in case of normal exit
	signal(SIGINT, handler); //in case of ctrl + C
}

void createClientPreliminaryQueue(){
	queueAtributes.mq_flags = 0;
	queueAtributes.mq_maxmsg = 10;
	queueAtributes.mq_msgsize = sizeof(struct serverBack);
	queueAtributes.mq_curmsgs = 0;
	myDescriptor = mq_open(myPath, O_CREAT | O_RDONLY, 0666, NULL );
	CHECK((mqd_t)-1 != myDescriptor);
	printf("Created own queue, id: %s\n", myPath);
}

void reportReadiness(){
	struct initReport clientReady;
	strcpy(clientReady.queue_id,myPath);
	clientReady.a ='a';
	CHECK( 0 <= mq_send(serverDescriptor, (char *)&clientReady, sizeof(struct initReport),0) );
	printf("reported\n");
}

void waitForResponse(){
	struct serverBack server_message;
	CHECK(mq_receive(myDescriptor, (char *)&server_message, 8192, NULL) >= 0); //sizeof(struct serverBack)
	printf("received text: %s and id: %d\n",server_message.mText, server_message.id);
	myClientId = server_message.id;
}

void infiniteLoop(){
	for (int i = 0; i< 100; i++){
		struct secondPhase_ClientWantsNumber clientWants;
		clientWants.id = myClientId;
		clientWants.a = 'b';
		//try to gain a dispatch
		CHECK((mq_send(serverDescriptor,
					  (char *) &clientWants, 
					  sizeof(struct secondPhase_ClientWantsNumber), 0)) >= 0);
		struct secondPhase_ServerGivesNumber serverGives;
		printf("Sent request\n");
		//wait for dispatch
		printf("Waiting for dispatch \n");
		CHECK((mq_receive(myDescriptor,
					  (char *) &serverGives, 
					  8192 , NULL)) >= 0); //sizeof(struct secondPhase_ServerGivesNumber) + 1
		struct secondPhase_ClientGivesNumber clientGives;
		clientGives.isPrime = isPrime(serverGives.num);
		clientGives.num = serverGives.num;
		clientGives.id = myClientId;
		clientGives.mType = CLIENT_READY;
		clientGives.a = 'c';
		//send response
		CHECK((mq_send(serverDescriptor,
					  (char *) &clientGives, 
					  sizeof(struct secondPhase_ClientGivesNumber), 0)) >= 0); // sizeof(struct secondPhase_ClientGivesNumber)
	}
	//sending message that I am exiting
	struct clientExits bye;
	bye.a = 'd';
	bye.id = myClientId;
	CHECK((mq_send(serverDescriptor,
				  (char *) &bye, 
				  sizeof(struct clientExits), 0)) >= 0); 
	
}

int main(int argc, char* argv[]){
	init(argc, argv);	
	createClientPreliminaryQueue();
	reportReadiness();
	waitForResponse();
	infiniteLoop();
}
