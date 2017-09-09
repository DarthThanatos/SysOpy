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

struct Client{
	struct Client *next;
	char client_id[50];
	struct sockaddr *address;
	socklen_t size;
};

static struct sockaddr_un sa_server;
struct sockaddr_un *sa_client;
static struct sockaddr_in net_server;
struct sockaddr_in *net_client;
int fd_net, fd_hwm = 0; 
bool shouldContinue = true;
struct Client *head = NULL;
struct ServerMsg serverMsg;

void unregister_client(struct ClientMsg clientMsg){
	printf("unregistering %s\n", clientMsg.client_id);
	unsigned long long now;
	struct Client *p = head, *a;
	if(head!= NULL){
		if(strcmp(head->client_id, clientMsg.client_id) ==0){
			printf("Deleted %s\n", clientMsg.client_id);
			head = head -> next;
			free(p);
			p = head;
			return;
		}
	}
	if (head == NULL) return;
	a = head;
	p = head -> next;
	while(p != NULL){
		if(strcmp(p->client_id, clientMsg.client_id)==0){
			printf("Deleted %s\n", clientMsg.client_id);
			a->next =p->next;
			free(p);
			p = a->next;
			return;
		}
		else {
			a = a->next;
			p = p->next;
		}
	}
}

struct Client *register_client(char client_id[50],
					struct sockaddr *address, 
					socklen_t sa_len){
	//search if client already exists
	struct Client *p;
	p = malloc(sizeof(struct Client));
	p->address = (struct sockaddr *)malloc(sizeof (struct sockaddr_in));
	((struct sockaddr_in *)p->address)->sin_family = AF_INET;
	((struct sockaddr_in *)p->address)->sin_port = ((struct sockaddr_in *)address)->sin_port;
	((struct sockaddr_in *)p->address)->sin_addr.s_addr = ((struct sockaddr_in *)address)->sin_addr.s_addr;
	strcpy(p->client_id, client_id);
	p->size = sa_len;
	if (head == NULL){
		head = p;
		head -> next = NULL;
	}
	else {
		p->next = head;
		head = p;
	}
	return p;
}

void sendToAll(struct ClientMsg clientMsg){
	for(struct Client *p = head; p!= NULL; p = p->next){ 
		printf("checking %s\n", p->client_id);
		int comp;
		if ( (comp=strcmp(p->client_id, clientMsg.client_id)) != 0){
			CHECK( sendto(fd_net, (void *)&serverMsg, sizeof(serverMsg), 0, p->address, p->size) != -1); 
			printf("All: Sent msg %s to %s from %s comp: %d\n", clientMsg.msg, p->client_id, clientMsg.client_id, comp);
		}
	} 
}

void sendToParticular(struct Client *p){
	CHECK( sendto(fd_net, (void *)&serverMsg, sizeof(serverMsg), 0, p->address, p->size) != -1); 
	printf("Particular: Sent msg %s to %s\n", serverMsg.msg, p->client_id);
}

void decideWhatToDo(struct ClientMsg clientMsg,struct sockaddr* net_client, socklen_t sa_len){
	if( strcmp(clientMsg.typeOfMsg,"register") == 0){
		struct Client *p = register_client(clientMsg.client_id,(struct sockaddr*)net_client, sa_len);
		sendToParticular(p);
	}
	if ( strcmp(clientMsg.typeOfMsg, "setMsg") == 0 ){
		strcpy(serverMsg.msg, clientMsg.msg);
		serverMsg.charactersInMsg = clientMsg.charactersInMsg;
		sendToAll(clientMsg);
	}
	if(strcmp(clientMsg.typeOfMsg,"exit") == 0){
		unregister_client(clientMsg);
	}
}

static void run_server(void) {       
	ssize_t nrecv;  
	char msg[MSG_SIZE];   
	socklen_t sa_len;
	CHECK(( fd_net = socket(AF_INET, SOCK_DGRAM, 0)) != -1);
	if( fd_net > fd_hwm)
		fd_hwm = fd_net;
	CHECK( bind(fd_net, (struct sockaddr*) &net_server, sizeof(net_server)) != -1);
	fd_set read_set,set;
	FD_ZERO(&set);
	FD_SET(fd_net, &set);
	struct ClientMsg clientMsg;
	while (shouldContinue) {  
		read_set = set;
		CHECK( select(fd_hwm + 1, &read_set, NULL, NULL, NULL) != -1) ; 
		for (int fd = 0; fd <= fd_hwm; fd++) {        
			if (FD_ISSET(fd, &read_set)) { 
				if(fd == fd_net){
					struct sockaddr_storage sa; 
					sa_len = sizeof(sa); 
					CHECK(( nrecv = recvfrom(fd_net,(void *)&clientMsg, sizeof(clientMsg), 0,(struct sockaddr *)&sa, &sa_len) )  != -1);  
					net_client = (struct sockaddr_in *)&sa;
					//printf("Client %s\n", clientMsg.client_id);
					decideWhatToDo(clientMsg,(struct sockaddr*)net_client, sa_len);
				}
			}
		}        
	}
}

void clean(){
	(void)unlink(sa_server.sun_path);
	CHECK(shutdown(fd_net, SHUT_RDWR) != -1);
	close(fd_net);
}

void handler(int signo){
	shouldContinue = false;
}

void init(){
	atexit(clean);
	signal(SIGINT,handler);
	serverMsg.charactersInMsg = 0;
	// setting internet socket address
	memset(&net_server, 0, sizeof net_server);
	net_server.sin_family = AF_INET; 
	CHECK( (net_server.sin_port = htons(SOCKET_PORT) )!= -1); //1025
	net_server.sin_addr.s_addr = htonl(INADDR_ANY);
}

int main(void) {
	init();
	run_server();  
}
