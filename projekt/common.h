#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#ifndef COMMON_H_
#define COMMON_H_
#define SPACE_SIZE 4
#define MSG_SIZE 100
#define SOCKET_PORT 8888
#define FILE_CAPACITY 500

struct ClientMsg{
	char client_id[50];
	char typeOfMsg[50];
	char msg[FILE_CAPACITY];
	int charactersInMsg;
};

struct ServerMsg{
	char msg[FILE_CAPACITY];
	int charactersInMsg;
};

void try(bool condition, char *msg){
	if(condition){
		printf("%s\n",msg);
		exit(0);
	}
}

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr,"%s %s:%d: ",__FILE__, __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \


#endif /* #ifndef COMMON_H_ */