#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "common.h"
#include <sys/select.h>
#include <signal.h>
#include <sys/time.h>

#define REGISTRATION_VALIDITY 5
//229 543 str. Rockhind
struct Client{
	struct Client *next;
	char client_id[50];
	int fd_client;
	unsigned long long lastRegistration;
};

fd_set set, read_set;  
struct sockaddr_un sap; 
struct sockaddr_in net;
bool shouldContinue = true;
int fd_skt, fd_net, fd_hwm = 0;
struct Client *head = NULL;

unsigned long long getTimeStamp(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long millisecondsSinceEpoch = (unsigned long long)(tv.tv_sec);
	return millisecondsSinceEpoch;
}

void setHighestFD(){
	for (struct Client *p = head; p!= NULL; p = p->next){
		if (p->fd_client > fd_hwm) fd_hwm = p->fd_client;
	}
	if(fd_net > fd_hwm)
		fd_hwm = fd_net;
	if (fd_skt > fd_hwm)
		fd_hwm = fd_skt;
}

void serverWakeUpActions(){
	printf("I woke up\n");
	unsigned long long now;
	struct Client *p = head, *a;
	while(head!= NULL){
		now = getTimeStamp();
		if(now - head->lastRegistration > REGISTRATION_VALIDITY){
			printf("Deleted and closed %s\n", p->client_id);
			head = head -> next;
			//in the code below the error checking has been omited, as there's no 
			//guarantee the client did not disconnect yet
			sendto(p -> fd_client, "unreg", 5, 0, NULL, 0);
			FD_CLR(p->fd_client, &set);
			FD_CLR(p->fd_client, &read_set);
			close(p->fd_client);
			if(p->fd_client == fd_hwm)
				fd_hwm--;
			free(p);
			p = head;
		}
		else break;
	}
	if (head == NULL) return;
	a = head;
	p = head -> next;
	while(p != NULL){
		if(now - p->lastRegistration > REGISTRATION_VALIDITY){
			printf("Deleted and closed %s\n", p->client_id);
			a->next =p->next;
			//in the code below the error checking has been omited, as there's no 
			//guarantee the client did not disconnect yet
			sendto(p -> fd_client, "unreg", 5, 0, NULL, 0); 
			FD_CLR(p->fd_client, &set);
			FD_CLR(p->fd_client, &read_set);
			close(p->fd_client);
			if(p->fd_client == fd_hwm)
				fd_hwm--;
			free(p);
			p = a->next;
		}
		else {
			a = a->next;
			p = p->next;
		}
	}
}

void register_client(char client_id[50],
					int fd_client){
	bool foundMatch = false;
	//search if client already exists
	struct Client *p;
	for (p = head; p != NULL; p = p->next){
		if (strcmp(p->client_id, client_id) == 0){
			foundMatch = true; 
			break;
		}
	}
	if(!foundMatch){
		p = malloc(sizeof(struct Client));
		p->fd_client = fd_client;
		strcpy(p->client_id, client_id);
		p->lastRegistration = getTimeStamp();
		if (head == NULL){
			head = p;
			head -> next = NULL;
		}
		else {
			p->next = head;
			head = p;
		}
		printf("registered %s\n", p->client_id);
	}
	else {
		// if the client already exists, p is set on their node
		p -> lastRegistration = getTimeStamp();
		printf("extended validation of %s\n", p->client_id);
	}
}

void sendToAll(struct ClientMsg clientMsg, char msg[MSG_SIZE], int actual_msg_size){
	for(struct Client *p = head; p!= NULL; p = p->next){
		if (strcmp(p->client_id, clientMsg.client_id) != 0){
			printf("sending to fd: %d\n",p->fd_client);
			CHECK( sendto(p -> fd_client, msg, actual_msg_size, 0, NULL, 0) != -1); 
		}
	} 
}

static bool run_server() {    
	int  fd_client, fd; 
	char msg[MSG_SIZE];   
	ssize_t nread;
	struct timeval timeout;
	CHECK(( fd_skt = socket(AF_UNIX, SOCK_STREAM, 0) )  != -1); 
	CHECK(( fd_net = socket(AF_INET, SOCK_STREAM, 0) )  != -1); 
	CHECK( bind(fd_skt, (struct sockaddr *)&sap, sizeof(sap)) != -1);  
	CHECK( bind(fd_net, (struct sockaddr *)&net, sizeof(net)) != -1);    
	CHECK( listen(fd_skt, SOMAXCONN) != -1)  ;     
	CHECK( listen(fd_net, SOMAXCONN) != -1)  ; 
	if (fd_skt > fd_hwm)       
		fd_hwm = fd_skt;    
	if (fd_net > fd_hwm)       
		fd_hwm = fd_net;
	FD_ZERO(&set);   
	FD_SET(fd_skt, &set);   
	FD_SET(fd_net,&set);
	struct ClientMsg clientMsg;
	while (shouldContinue) { 
		read_set = set;   
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;    
		//setHighestFD();
		CHECK( select(fd_hwm + 1, &read_set, NULL, NULL, &timeout) != -1) ; 
		serverWakeUpActions(); 
		for (fd = 0; fd <= fd_hwm; fd++) {        
			if (FD_ISSET(fd, &read_set)) { 
				if(fd == fd_skt){
					CHECK(( fd_client = accept(fd_skt, NULL, 0) )   != -1);
					FD_SET(fd_client, &set);                  
					if (fd_client > fd_hwm)               
						fd_hwm = fd_client; 
				}
				else {
					if(fd == fd_net){
						CHECK(( fd_client = accept(fd_net, NULL, 0) )   != -1);
						FD_SET(fd_client, &set);                  
						if (fd_client > fd_hwm)               
							fd_hwm = fd_client; 
					}
					else {
						CHECK(( nread = recvfrom(fd,(void *)&clientMsg, sizeof(clientMsg), 0,NULL, NULL) )  != -1);  
						//CHECK( (nread = read(fd, buf, sizeof(buf)) )  != -1);
						if (nread == 0) {             
							FD_CLR(fd, &set);                  
							if (fd == fd_hwm)                
								fd_hwm--;                 
							CHECK(close(fd) != -1);          
						}                 
						else {   
							printf("active fd: %d\n",fd);
							register_client(clientMsg.client_id, fd);                 
							printf("Server got \"%s\" from %s\n",  clientMsg.msg, clientMsg.client_id);     
							sprintf(msg, "%s: %s",clientMsg.client_id, clientMsg.msg);
							sendToAll(clientMsg,msg,sizeof(msg));
						}             
					}
				}
			}
		}
	}   
	CHECK( close(fd_net) != -1);    
	return true;
}

void clean(){
	(void)unlink(sap.sun_path);
	CHECK(shutdown(fd_net, SHUT_RDWR) != -1);
	close(fd_net);
}

void handler(int signo){
	shouldContinue = false;
}

void init(int argc, char *argv[]){
	if (argc != 3){
		printf("Wrong usage ./server path port\n");
		exit(1);
	}
	atexit(clean);
	signal(SIGINT,handler);
	// setting internet socket address
	memset(&net, 0, sizeof net);
	net.sin_family = AF_INET; 
	CHECK( (net.sin_port = htons(atoi(argv[2])) )!= -1); //1025
	net.sin_addr.s_addr = htonl(INADDR_ANY);
	//setting unix socket address
	strncpy(sap.sun_path, argv[1], strlen(argv[1]));  
	printf("%s\n", sap.sun_path); 
	sap.sun_family = AF_UNIX;  
}

int main(int argc, char *argv[]) {   
	init(argc,argv);
	run_server();  
}