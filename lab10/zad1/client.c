#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "common.h"
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#include <pthread.h>

#define MAX_STR_LEN 50000

char *client_id;
char *path;
int ip_address;   
int fd_skt;
int server_port;
struct sockaddr_in net;
struct sockaddr_in myaddr_net;
struct sockaddr_un local;
struct sockaddr_un myaddr; 
struct sockaddr *abstract;
// ^ abstract address to  serve for both unix and internet socekts
bool local_server_active = true;
bool shouldContinue = true;
struct ClientMsg clientMsg;
char backmsg[MSG_SIZE];     

bool typedExit(){
	char *stop = "exit";
	bool equalsExit = true;
	int i;
	for ( i = 0; clientMsg.msg[i] != '\0'; i++){
		if(i < 4 && stop[i] != clientMsg.msg[i])
			equalsExit = false;
		if (clientMsg.msg[i] == '\n'){
			clientMsg.msg[i] = '\0';
			break;
		}
	}
	if(i != 4) equalsExit = false;
	return equalsExit;
}

void *thread_IO(void *parent_id){
	while(true){
		printf("Type your message or exit to exit: ");
		fgets(clientMsg.msg, MAX_STR_LEN, stdin);
		if(typedExit()){
			pthread_kill((pthread_t)parent_id, SIGINT);
			break;
		}
		pthread_kill((pthread_t)parent_id, SIGUSR1);
	}
	return NULL;
}

void establishMyPort(){
	//this function increments port number of a client in case 
	//it is already in use
	int err;
	while(true){
		err = bind(fd_skt, (struct sockaddr*) &myaddr_net,  sizeof(myaddr_net));
		if(err == -1){
			if (errno == EADDRINUSE){
				//expected situation
				myaddr_net.sin_port ++;
			}
			else {
				//another unfortunate event has happend...
				fprintf(stderr,"%s %s:%d: ",__FILE__, __func__, __LINE__); 
				perror("bind: "); 
				exit(-1); 
			}
		}
		else break;
	}
}

static void run_client() {   
	if(local_server_active){
		CHECK( (fd_skt = socket(AF_UNIX, SOCK_DGRAM, 0) )  != -1) ;
		abstract = (struct sockaddr *)&local;    
		CHECK( bind(fd_skt, (struct sockaddr*) &myaddr,  sizeof(myaddr)) != -1);   
	}
	else {
		CHECK((fd_skt = socket(AF_INET, SOCK_DGRAM, 0)) != -1);
		abstract = (struct sockaddr *)&net;  
		establishMyPort();
	}
	fd_set read_set,set;
	socklen_t sa_len;
	FD_ZERO(&set);
	FD_SET(fd_skt, &set);  
	while(shouldContinue){
		read_set = set;
		select(fd_skt + 1,&read_set,NULL,NULL,NULL);
		if(FD_ISSET(fd_skt,&read_set)){
			recvfrom(fd_skt, backmsg, sizeof(backmsg), 0,abstract, &sa_len);
			printf("Got \"%s\"\n", backmsg);  
		}
	}
}

void clean(){
	CHECK( close(fd_skt) != -1) ;
	(void) unlink(client_id);
}

void handler(int signo){
	if(signo == SIGINT){
		exit(0);
	}
	else{
		CHECK( sendto(fd_skt, (void *)&clientMsg, sizeof(clientMsg), 0,  abstract, sizeof(*abstract)) != -1); 
	}
}

void init(int argc, char* argv[]){
	if (argc > 5 || argc < 3){
		printf("Wrong usage ./client client_id server_type [local == 0 | remote == 1] location [path | (ip and port) ] \n");
		exit(1);
	}
	atexit(clean);
	signal(SIGINT,handler);
	signal(SIGUSR1,handler);
	client_id = argv[1];
	if(atoi(argv[2])) {
		memset(&net, 0, sizeof net);
		CHECK((net.sin_port = htons(atoi(argv[4])) )!= -1); //8888
		net.sin_addr.s_addr = inet_addr(argv[3]); // "127.0.0.1"
		net.sin_family = AF_INET;
		local_server_active = false;
		myaddr_net.sin_family = AF_INET;
		myaddr_net.sin_addr.s_addr = inet_addr(argv[3]);
		myaddr_net.sin_port = htons(atoi(argv[4])+1);
		printf("port: %d\n", myaddr_net.sin_port);
		printf("path: %d\n", myaddr_net.sin_addr.s_addr);
	}
	else {
		//local server is already active by default, hence we do not set local_server_active flag
		local.sun_family = AF_UNIX;
		printf("working %s\n", argv[3]);
		strcpy(local.sun_path, argv[3]); 

		myaddr.sun_family = AF_UNIX;
		strcpy(myaddr.sun_path, client_id);
		printf("path: %s\n", myaddr.sun_path);
	}
	strcpy(clientMsg.client_id, client_id);
	pthread_t child_id;
	pthread_create(&child_id, NULL, &thread_IO, (void *) pthread_self());
}

int main(int argc, char *argv[]) {      
	init(argc,argv);
	run_client();  
}