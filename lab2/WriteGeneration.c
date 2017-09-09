#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

void createRandomWriteFile(char *path, int recordSize, int recordsAmount){
	int fin,i,j,random;
	char record[recordSize];
	if( (fin= open(path, O_RDWR | O_CREAT | O_TRUNC | O_APPEND,0600)) == -1){
		printf("Could not open the file");
		return;
	}
	random = time(NULL); srand(random);
	for (j = 0; j < recordsAmount; j++){
		for (i = 1; i< recordSize - 1; i++)
			record[i] = (char) (rand()%256);
		record[0] = (char) (rand()%(10) + 48);
		record[recordSize - 1] = '\n';
	    if(write (fin, record, recordSize) < 0){
	        printf("Error in write\n");
	        //printf("%s", strerror(errno));
	        return;
	    }
	}
	close(fin);

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
	createRandomWriteFile(path, recordSize, recordsAmount);
}
