#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define pathlen 1000
char *path;
char command [pathlen];

void parseCommand(){
    strcat(command, "ls -l | grep ^d > ");
    strcat(command,path);
}

void init(int argc, char *argv[]){
	if (argc != 2){
		printf("Wrong usage!\n");
		exit(1);
	}
	path = argv[1];
}

int main(int argc, char*argv[]){
	FILE *fp;
	init(argc,argv);
	parseCommand();
	fp = popen(command, "r");
	if (fp == NULL){
	    printf("error!!!\n");
	    return -1;	
	}
	//while (fgets(path,pathlen, fp) != NULL)
	//	printf("%s", path);
	pclose(fp);
	return 0;
}
