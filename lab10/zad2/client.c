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
struct sockaddr_un local;
bool local_server_active = true;
bool shouldContinue = true;
struct ClientMsg clientMsg;
struct sockaddr *sap;
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

void connect_to_server(){
	if(local_server_active){
		CHECK( (fd_skt = socket(AF_UNIX, SOCK_STREAM, 0) )  != -1) ;
		sap = (struct sockaddr *) &local;  
	}
	else {
		sap = (struct sockaddr *) &net;
		CHECK((fd_skt = socket(AF_INET, SOCK_STREAM, 0)) != -1);
	}
	while (connect(fd_skt, sap, sizeof(*sap)) == -1){
		if (errno == ENOENT) {    
			sleep(1);              
			continue;            
		}           
		else  {   
			fprintf(stderr,"%s %s:%d: ",__FILE__, __func__, __LINE__); 
			perror("connect: "); 		
			exit(1);  
		}
	}  
}

static bool run_client() {  
	fd_set read_set,set;
	connect_to_server();
	FD_ZERO(&set);
	FD_SET(fd_skt, &set); 
	while(shouldContinue){  
		read_set = set;
		select(fd_skt + 1,&read_set,NULL,NULL,NULL);
		if(FD_ISSET(fd_skt,&read_set)){             
			//read(fd_skt, backmsg, sizeof(backmsg));
			memset(backmsg, '\0', MSG_SIZE);
			recvfrom(fd_skt, backmsg, sizeof(backmsg), 0,NULL, NULL);
			if (strcmp(backmsg, "unreg") == 0){
				printf("Unregistered...\n");
				exit(0);
			}
			else 
				printf("Got \"%s\"\n", backmsg);  
		}
	}
	return true;
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
		 //write(fd_skt, clientMsg.msg, strlen(clientMsg.msg) + 1 );  
		CHECK( sendto(fd_skt, (void *)&clientMsg, sizeof(clientMsg.msg), 0,  NULL,0) != -1); 
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
		CHECK((net.sin_port = htons(atoi(argv[4])) )!= -1); //1025
		net.sin_addr.s_addr = inet_addr(argv[3]); // "127.0.0.1"
		net.sin_family = AF_INET;
		local_server_active = false;
		printf("net\n");
	}
	else {
		//local server is already active
		local.sun_family = AF_UNIX;
		strcpy(local.sun_path, argv[3]);  
	}
	strcpy(clientMsg.client_id, client_id);
	pthread_t child_id;
	pthread_create(&child_id, NULL, &thread_IO, (void *) pthread_self());
}

int main(int argc, char *argv[]) {      
	init(argc,argv);
	run_client();  
}