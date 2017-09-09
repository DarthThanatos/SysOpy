#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

void createRandomFWriteFile(char *path, int recordSize, int recordsAmount){
	int i,j,random;
	char record[recordSize];
	remove(path);
	FILE *recordsFile = fopen(path,"a");
	if(recordsFile == NULL){
		printf("Could not open the file");
		return;
	}
	random = time(NULL); srand(random);
	for (j = 0; j < recordsAmount; j++){
		for (i = 1; i< recordSize - 1; i++)
			record[i] = (char) (rand()%256);
		record[0] = (char) (rand()%(10) + 48);
		fwrite(record,1,recordsAmount - 1,recordsFile);
		fwrite("\n",1,1,recordsFile);
	}
	fclose(recordsFile);
}

int main(int argc, char *argv[]){
	int recordSize, recordsAmount;
	char * path;
	if(argc != 4){
		printf("Wrong usage!!");
		return -1;
	}
	path = argv[1];
	recordSize = atoi(argv[2]);
	recordsAmount = atoi(argv[3]);
	if (recordSize < 0){
		printf("record size cannot be negative");
		return -1;
	}
	if (recordsAmount < 0){
		printf("records amount cannot be negative");
		return -1;
	}
	createRandomFWriteFile(path, recordSize, recordsAmount);
}
