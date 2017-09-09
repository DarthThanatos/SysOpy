#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define MAX_STR_LEN 50000
char *fifoName;

char parsedMsg[MAX_STR_LEN];

void formulateMsg(char *msg){
    struct tm * timeinfo;
    time_t rawtime;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char hour[2], myPid[10];
    sprintf(hour,"%d",timeinfo->tm_hour);
    sprintf(myPid,"%d",getpid());
    //PID - HOUR - MSG
    strcat(parsedMsg,myPid);
    strcat(parsedMsg," ");
    strcat(parsedMsg,hour);
    strcat(parsedMsg," ");
    strcat(parsedMsg,msg); 
    strcat(parsedMsg,"\n");
}

void init(int argc, char *argv[]){
    if (argc != 2){
	printf("wrong usage\n");
	exit(1);
    }
    fifoName = argv[1];
}


void setMsg(char *msg){
	fgets(msg, MAX_STR_LEN, stdin);
}

int main(int argc, char *argv[]){
    init(argc,argv); 
    FILE * fifo;
    mknod(fifoName, S_IFIFO | 0666, 0);
    char msg[MAX_STR_LEN];
    while(1){
       fifo = fopen(fifoName, "w");
       printf("type message to send: ");
       //scanf("%s",msg);
       setMsg(msg);
       formulateMsg(msg);
       fwrite(parsedMsg,1,MAX_STR_LEN,fifo);
       fclose(fifo);
       printf("Sent %s\n",parsedMsg);
       parsedMsg[0] = '\0';
    }
    return 0;

}
