#include <string.h>
#include <stdlib.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "common.h"
#include <sys/select.h>
#include <signal.h>
#include <sys/time.h>

#define REGISTRATION_VALIDITY 5

struct Client{
	bool isLocal;
	struct Client *next;
	char client_id[50];
	struct sockaddr *address;
	unsigned long long lastRegistration;
	socklen_t size;
};

static struct sockaddr_un sa_server;
struct sockaddr_un *sa_client;
static struct sockaddr_in net_server;
struct sockaddr_in *net_client;
int fd_skt, fd_net, fd_hwm = 0; 
bool shouldContinue = true;
struct Client *head = NULL;

unsigned long long getTimeStamp(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long millisecondsSinceEpoch = (unsigned long long)(tv.tv_sec);
	return millisecondsSinceEpoch;
}

void serverWakeUpActions(){
	printf("I woke up\n");
	unsigned long long now;
	struct Client *p = head, *a;
	while(head!= NULL){
		now = getTimeStamp();
		if(now - head->lastRegistration > REGISTRATION_VALIDITY){
			printf("Deleted %s\n", p->client_id);
			head = head -> next;
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
			printf("Deleted %s\n", p->client_id);
			a->next =p->next;
			free(p);
			p = a->next;
		}
		else {
			a = a->next;
			p = p->next;
		}
	}
}

void register_client(bool isLocal,
					char client_id[50],
					struct sockaddr *address, socklen_t sa_len){
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
		p->isLocal = isLocal;
		if(isLocal){
			p->address = (struct sockaddr *)malloc(sizeof (struct sockaddr_un));
			strcpy(((struct sockaddr_un *)p->address)->sun_path, ((struct sockaddr_un*) address)->sun_path);
			((struct sockaddr_un *)p->address)->sun_family = AF_UNIX;
		}
		else{
			p->address = (struct sockaddr *)malloc(sizeof (struct sockaddr_in));
			((struct sockaddr_in *)p->address)->sin_family = AF_INET;
			((struct sockaddr_in *)p->address)->sin_port = ((struct sockaddr_in *)address)->sin_port;
			((struct sockaddr_in *)p->address)->sin_addr.s_addr = ((struct sockaddr_in *)address)->sin_addr.s_addr;
		}
		strcpy(p->client_id, client_id);
		p->lastRegistration = getTimeStamp();
		p->size = sa_len;
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
			if(p->isLocal){
				printf("client: %s path: %s\n",p->client_id, ((struct sockaddr_un *)p->address)->sun_path);
				CHECK( sendto(fd_skt, msg, actual_msg_size, 0, p->address, p->size) != -1); 
				printf("Sent msg to %s\n", p->client_id);
			}
			else{
				printf("client: %s port: %d\n",p->client_id, ((struct sockaddr_in *)p->address)->sin_port);
				printf("path: %d\n", ((struct sockaddr_in *)p->address)->sin_addr.s_addr);
				CHECK( sendto(fd_net, msg, actual_msg_size, 0, p->address, p->size) != -1); 
				printf("Sent msg to %s\n", p->client_id);
			}
		}
	} 
}

static void run_server(void) {       
	ssize_t nrecv;  
	char msg[MSG_SIZE];   
	socklen_t sa_len;
	/*struct for alerting server to do cleanup*/
	struct timeval timeout;
	CHECK(( fd_skt = socket(AF_UNIX, SOCK_DGRAM, 0)) != -1);
	if(fd_skt > fd_hwm) 
		fd_hwm = fd_skt;
	CHECK( bind(fd_skt, (struct sockaddr *)&sa_server, sizeof(sa_server)) != -1);    
	
	CHECK(( fd_net = socket(AF_INET, SOCK_DGRAM, 0)) != -1);
	if( fd_net > fd_hwm)
		fd_hwm = fd_net;
	CHECK( bind(fd_net, (struct sockaddr*) &net_server, sizeof(net_server)) != -1);
	fd_set read_set,set;
	FD_ZERO(&set);
	FD_SET(fd_skt, &set);
	FD_SET(fd_net, &set);
	struct ClientMsg clientMsg;
	while (shouldContinue) {  
		read_set = set;  
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		CHECK( select(fd_hwm + 1, &read_set, NULL, NULL, &timeout) != -1) ; 
		serverWakeUpActions();
		for (int fd = 0; fd <= fd_hwm; fd++) {        
			if (FD_ISSET(fd, &read_set)) { 
				if(fd == fd_skt){    
					struct sockaddr_storage sa; 
					sa_len = sizeof(sa);  
					CHECK(( nrecv = recvfrom(fd_skt, (void *) &clientMsg, sizeof(clientMsg), 0,(struct sockaddr *)&sa, &sa_len) )  != -1);  
					sprintf(msg, "%s: %s",clientMsg.client_id, clientMsg.msg);
					printf("%s\n",msg);
					sa_client = (struct sockaddr_un *)&sa;
					register_client(true, clientMsg.client_id,(struct sockaddr *)sa_client,sa_len);
					sendToAll(clientMsg,msg,sizeof(msg));
				}
				if(fd == fd_net){
					struct sockaddr_storage sa; 
					sa_len = sizeof(sa); 
					CHECK(( nrecv = recvfrom(fd_net,(void *)&clientMsg, sizeof(clientMsg), 0,(struct sockaddr *)&sa, &sa_len) )  != -1);  
					sprintf(msg, "%s: %s",clientMsg.client_id, clientMsg.msg);
					printf("%s\n",msg);
					net_client = (struct sockaddr_in *)&sa;
					register_client(false, clientMsg.client_id,(struct sockaddr*)net_client, sa_len);
					sendToAll(clientMsg,msg, sizeof(msg));
				}
			}
		}        
	}
	CHECK( close(fd_skt) != -1);   
	exit(EXIT_SUCCESS);
}

void clean(){
	(void)unlink(sa_server.sun_path);
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
	memset(&net_server, 0, sizeof net_server);
	net_server.sin_family = AF_INET; 
	CHECK( (net_server.sin_port = htons(atoi(argv[2])) )!= -1); //1025
	net_server.sin_addr.s_addr = htonl(INADDR_ANY);
	//setting unix socket address
	strncpy(sa_server.sun_path, argv[1], strlen(argv[1]));  
	sa_server.sun_family = AF_UNIX;  
}

int main(int argc, char *argv[]) {
	init(argc,argv);
	run_server();  
}
