#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

char* path;
int mode;
int descriptorToWrite;
int descriptorToRead;
char changeByte;


bool cmp(char *firstString, char *secondString){
	int firstLen = strlen(firstString), secondLen = strlen(secondString), i;
	if(firstLen!=secondLen) return false;
	for( i =0; i< firstLen; i++){
		if(firstString[i] != secondString[i])
			return false;
	}
	return true;
}

int getByteAndChar(char *msg){
	int byteNoInt;
	char byteNoChar [50];
	char cmpNoChar [50];
	printf("%s", msg);
	scanf("%s %c", byteNoChar, &changeByte);
	byteNoInt = atoi(byteNoChar);
	sprintf(cmpNoChar,"%d",byteNoInt);
	if(!cmp(cmpNoChar,byteNoChar)) return -1;
	return byteNoInt;
}

int getByte(char *msg){
	int byteNoInt;
	char byteNoChar [50];
	char cmpNoChar [50];
	printf("%s", msg);
	scanf("%s", byteNoChar);
	byteNoInt = atoi(byteNoChar);
	sprintf(cmpNoChar,"%d",byteNoInt);
	if(!cmp(cmpNoChar,byteNoChar)) return -1;
	return byteNoInt;
}

typedef struct Node{
	struct Node *next;
	int byteNo;
	int pid;
	char *io;
	int posInFile;
	int len;
}Node;

Node *head;

void append(int byteNo, int pid, int pos, char *io, int len){
	if (head == NULL){
		head = malloc(sizeof (Node));
		head->next = NULL;
	}
	else{
		Node *p = malloc (sizeof (Node));
		p->next = head;
		head = p;
	}
	head->byteNo = byteNo;
	head->pid = pid;
	head->posInFile = pos;
	head ->io = io;
	head->len = len;

}


void updateList(char *io, int byteNo){
	Node *p;
	bool nodeOccurred = false;
	for (p=head; p!=NULL; p=p->next) {
		if(p->byteNo == byteNo &&
		   p->pid == getpid() &&
		   cmp(p->io,io)) {
				nodeOccurred = true;
				break;
		}
	}
	if(!nodeOccurred){
		FILE *listOfPids = fopen("listOfPids","a");
		if(listOfPids == 0){
			printf("read error");
			return;
		}
		int pos = ftell(listOfPids);
		fprintf(listOfPids, "byte no %d is locked to %s by pid %d\n",byteNo, io, getpid());
		int len = ftell(listOfPids) - pos;
		append(byteNo,getpid(),pos,io,len);
		fclose(listOfPids);
		printf("==================================\n"
				"added, len: %d\n"
				"=================================\n",len);
	}
	else
		printf("==================================\n"
				"already occurred\n"
				"=================================\n");
}

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len){
    struct flock lock;
    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;
    return(fcntl(fd,cmd,&lock));
}

#define readw_lock(fd, offset, whence, len) \
	lock_reg(fd, F_SETLKW, F_RDLCK,offset, whence, len)
#define writew_lock(fd, offset, whence, len) \
	lock_reg(fd, F_SETLKW, F_WRLCK,offset, whence, len)
#define un_lock(fd, offset, whence, len) \
	lock_reg(fd,F_SETLK, F_UNLCK, offset, whence, len)

void deleteNode( Node *p){
        if(p==NULL) return;
	Node *a = head;
	if(a==p){
	   head = head -> next;
	   free(p);
	}
	else{
	   for (a = head; a->next !=NULL && a->next != p; a = a->next);
	   if(a->next == p){
	      a->next = p->next;
	      free(p);
	   } 
	}
}

void deleteLock(int byteNo){
	Node *p = head;
	while (p!=NULL){
		bool nodeOccurred = false;
		for (p = head; p!=NULL; p = p->next){
			if(p->byteNo == byteNo &&
			   p->pid == getpid()){
				nodeOccurred = true;
				break;
			}
		}
		if(nodeOccurred){
			FILE *listOfPids = fopen("listOfPids","r+");
			char *deleted = malloc(sizeof(p->len));
			int i;
			for (i =0; i< p->len - 1; i++) deleted[i] = ' ';
			deleted[p->len] = '\0';
			fseek(listOfPids,p->posInFile,SEEK_SET);
			fprintf(listOfPids,deleted,p->len);
			fclose(listOfPids);
			deleteNode(p);
			printf("===================================\n"
					"Unlocked\n"
					"==================================\n");
		}
		else{
			printf("===================================\n"
					"Nothing to unlock \n"
					"==================================\n");
		}
	}
}

void setReadLock(){
	int byteNo;
	byteNo = getByte("Type number of byte to lock (reading mode): ");
	if(byteNo < 0){
		printf("===============================\n"
				"wrong input\n"
				"==============================\n");
		return;
	}
	if(readw_lock(descriptorToRead, byteNo, SEEK_SET, 1) < 0){
		printf("readw_lock error\n");
		return;
	}
	updateList("read", byteNo);
}

void setWriteLock(){
	int byteNo;
	if( (byteNo = getByte("Type number of byte to lock (writing mode): ")) < -1){
		printf("===============================\n"
				"wrong input\n"
				"==============================\n");
		return;
	}
	if(writew_lock(descriptorToWrite, byteNo, SEEK_SET, 1) < 0){
		printf("writew_lock error\n");
		return;
	}
	updateList("write",byteNo);
}

void showBlocked(){
	printf("---------------------------------------\n");
	system("cat listOfPids");
	printf("---------------------------------------\n");
}


void unlock(){
	int byteNo;
	byteNo = getByte("Type number of byte to unlock: ");
	if(byteNo < 0){
		printf("===============================\n"
				"wrong input\n"
				"==============================\n");
		return;
	}
	if(un_lock(descriptorToWrite, byteNo, SEEK_SET, 1) < 0){
		printf("un_lock error\n");
		return;
	}
	deleteLock(byteNo);
}

void readFromFile(){
	int byteNo;
	char readByte;
	byteNo = getByte("Type number of byte to read: ");
	if(byteNo < 0){
		printf("===============================\n"
				"wrong input\n"
				"==============================\n");
		return;
	}
	lseek(descriptorToRead, byteNo, SEEK_SET);
	if(read(descriptorToRead,&readByte,1)<0){
		printf("read error\n");
		return;
	}
	printf("===================================\n"
			"Read byte: %c %d\n"
			"==================================\n", readByte, byteNo);
}

void writeToFile(){
	int byteNo;
	byteNo = getByteAndChar("Type number of byte to change and then the literal: ");
	if(byteNo < 0){
		printf("===============================\n"
				"wrong input\n"
				"==============================\n");
		return;
	}
	lseek(descriptorToWrite, byteNo, SEEK_SET);
	if(write(descriptorToWrite,&changeByte,1)<0){
		printf("write error\n");
		return;
	}

	printf("===================================\n"
			"Overrided %d\n"
			"==================================\n",byteNo);
}

int interactiveMode(int argc, char *argv[]){
	bool shouldContinue = true;
	if(argc != 2){
		printf("Wrong usage!");
		return -1;
	}
	path = argv[1];
	descriptorToWrite = open(path,O_WRONLY | O_CREAT);
	descriptorToRead = open (path, O_RDONLY);
	if(descriptorToRead < 0 ||
			descriptorToWrite < 0){
				printf("could not open file %s", path);
				mode = 0;
				return -1;
	}

	while(shouldContinue){
		mode = getByte("Select One from modes below:"
				"\n0 - exit"
				"\n1 - set lock on reading"
				"\n2 - set lock on writing"
				"\n3 - showing list of blocked segments"
				"\n4 - unlock the lock"
				"\n5 - try to read selected byte from a file"
				"\n6 - change selected byte: ");
		switch(mode){
			case 0:
				shouldContinue = false;
				break;
			case 1:
				setReadLock();
				break;
			case 2:
				setWriteLock();
				break;
			case 3:
				showBlocked();
				break;
			case 4:
				unlock();
				break;
			case 5:
				readFromFile();
				break;
			case 6:
				writeToFile();
				break;
			default:
				printf("===============================\n"
						"wrong option\n"
						"==============================\n");
				break;
		}
	}
	close(descriptorToRead);
	close(descriptorToWrite);
	return 0;
}

int main(int argc, char *argv[]){
	return interactiveMode(argc,argv);
}
