#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/times.h>
#include <unistd.h>
#include <string.h>


#define cmdLen  50
char *cmd;

char *getCMD(char *command,char *argv[], int argc){
    int i;
    cmd = malloc (cmdLen);
    for(i =0; i< strlen(cmd); i++) cmd[i] = ' '; cmd[i] = '\0';	
    strcat(cmd,command);
    strcat(cmd," ");
    for (i = 0; i < argc; i++){
	strcat(cmd, argv[i]);
	strcat(cmd, " "); 	
    }
    return cmd;
}

void mvGeneratedToSafe(char *path){
   char *copyArgs[] = {path, "beforeSort.txt"};
   system(getCMD("cp", copyArgs, 2));
}

void mvGeneratedBack(char *path, char *dest){
   char *copyArgs[] = {path,dest};
   system(getCMD("cp", copyArgs,2));
   copyArgs[0] = "beforeSort.txt"; copyArgs[1] = path;
   system(getCMD("cp",copyArgs,2));
   system(getCMD("rm", copyArgs, 1));
}

void sortSys(int recordSize, char *path){
   printf("sortSys\n");
   int i, j;
   char currentRecord[recordSize], comparedRecord[recordSize];
   char checking;
   int file = open(path,O_RDWR,0600);
   mvGeneratedToSafe(path);
   if(file == -1){
		 printf("could not find %s", path);
		 return;
   }
   FILE *end = fopen(path, "r");
   for (i = 1;  (checking = getc(end))!= EOF; i++){
		j = i;
		lseek(file, i * recordSize, SEEK_SET);
		read(file, currentRecord,recordSize);
		while(j>0) {
			lseek(file,  (j-1) * recordSize, SEEK_SET);
			read(file, comparedRecord,recordSize);
			if((int) comparedRecord[0] <= (int) currentRecord[0]) break;
			lseek(file, (j-1) * recordSize, SEEK_SET);
			write(file, currentRecord,recordSize);
			lseek(file, (j) * recordSize, SEEK_SET);
			write(file, comparedRecord,recordSize);
			j--;
		}
		fseek(end, (i+1) * recordSize, SEEK_SET);
   }
   close(file);
   fclose(end);
   mvGeneratedBack(path,"afterSortSys.txt");
}

void swapStrings(char *firstString[], char *secondString[]){
    char **addressToSwap = firstString;
    *addressToSwap = *firstString;
    *firstString = *secondString;
    *secondString = *addressToSwap;
}

void sortLib(int recordSize, char *path){
   printf("sortLib\n");
   int i, j;
   char currentRecord[recordSize], comparedRecord[recordSize];
   char checking;
   FILE *file = fopen(path,"r+");
   mvGeneratedToSafe(path);
   if(file == NULL){
         printf("could not find %s", path);
         return;
    }
   for (i = 1; (checking = getc(file)) != EOF; i++){
		j = i;
		fseek(file, i * recordSize, SEEK_SET);
		fread(currentRecord, recordSize, 1, file);
		while(j>0) {
			fseek(file,  (j-1) * recordSize, SEEK_SET);
			fread(comparedRecord, recordSize,1,file);
			if((int) comparedRecord[0] <= (int) currentRecord[0]) break;
			fseek(file, (j-1) * recordSize, SEEK_SET);
			fwrite(currentRecord, recordSize,1,file);
			fseek(file, (j) * recordSize, SEEK_SET);
			fwrite(comparedRecord, recordSize, 1, file);
			j--;
		}
		fseek(file, (i+1) * recordSize, SEEK_SET);
   }
   fclose(file);
   mvGeneratedBack(path,"afterSortLib.txt");
}

void sort(char *mode, int recordSize, char *path){
   FILE *filePtr;
   struct tms sortTime, start;
   long clockTackts = sysconf(_SC_CLK_TCK);
   times(&start);
   if (mode[0] == 's') sortSys(recordSize,path);
   else sortLib(recordSize, path);
   times(&sortTime);
   filePtr = fopen("wyniki.txt","a");
   fprintf(filePtr,"\nsorting via %s fun for record size = %d\nsys time: %7.2f\nuser time: %7.2f\n\n",
		   mode,
		   recordSize,
		   (sortTime.tms_stime - start.tms_stime)/(double)clockTackts,
		   (sortTime.tms_utime - start.tms_utime)/(double)clockTackts);
   fclose(filePtr);
}

bool modeCorrect(char *mode){
	int i;
	char *sys = "sys", *lib = "lib";
	if(strlen(mode)!=3){
		return false;
	}
	for (i=0; i< 3; i++){
		if(mode[i]!=sys[i] && mode[i]!= lib[i])return false;
	}
	return true;
}

int main(int argc, char * argv[]){
	if (argc != 4){
		printf("Wrong usage!!");
		return -1;
	}
	char *path = argv[1];
	int recordSize = atoi(argv[2]);
	char *mode = argv[3];
	if (recordSize<0){
		printf("record size cannot be negative");
		return -1;
	}
	if(!modeCorrect(mode)){
		printf("wrong mode: neither sys nor lib");
		return -1;
	}
	sort(mode,recordSize,path);
	return 0;
}
