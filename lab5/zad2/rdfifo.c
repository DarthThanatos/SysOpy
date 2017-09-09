#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

char * FIFO_NAME;
#define MAX_STR_LEN  50000

void init(int argc, char *argv[]){
    if (argc != 2){
	printf("Wrong usage\n");
	exit(1);
    }
    FIFO_NAME = argv[1];
}

int main(int argc, char*argv[]){
    init(argc,argv);
    FILE * fifo;
    mknod(FIFO_NAME, S_IFIFO | 0666, 0);
    while(1){
        fifo = fopen(FIFO_NAME, "r");
        char msg[MAX_STR_LEN];
        fread(msg, 1, MAX_STR_LEN, fifo);
        printf("%s\n",msg);
    }
    return 0;
}
