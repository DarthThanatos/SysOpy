#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

bool vOption = false;
bool wOption = false;
bool exitFromProgram = false;

bool cmp(char *firstString, char *secondString){
	int firstLen = strlen(firstString), secondLen = strlen(secondString), i;
	if(firstLen!=secondLen) return false;
	for( i =0; i< firstLen; i++){
		if(firstString[i] != secondString[i])
			return false;
	}
	return true;
}

char *concatenatePath(char *path, char * dir){
	int pathlen = strlen(path), i,j,range;
	int dirlen = strlen(dir);
	char * res;
	bool slashExists = (path[pathlen-1] == '/');
	if(slashExists) res = malloc(pathlen + dirlen);
	else res = malloc(pathlen + 1 + dirlen);
	for (i = 0; i < pathlen; i++)
	   res[i] = path[i];
	if (!slashExists) {
	    res[i++] = '/';
	    range = dirlen + pathlen + 1;
	}
	else
	    range = dirlen + pathlen;
	for (j=0; i<range; i++, j++)
	    res[i] = dir[j];
	res[i] = '\0';
	return res;
}

void seeTree(char *path){
	DIR *dir;
	struct dirent *dirDesc;
	struct stat statbuf;
	dir = opendir(path);
	int amountOfFiles = 0;
	int childrenAmountOfFiles = 0;
	int pid,i;
	int childrenAmount = 0;
	if(dir != NULL){
		while((dirDesc = readdir(dir))){
		    int status = lstat(concatenatePath(path,dirDesc->d_name),&statbuf);
		    if(status == -1){
				continue;
		    }
	    	if (S_ISDIR(statbuf.st_mode)){
		    	if (cmp(dirDesc->d_name,".") ||
		    		cmp(dirDesc->d_name,".."))
		    		continue;
		    	if((pid = fork()) < 0){
		    		printf("fork error\n");
		    		return;
		    	}
		    	else{
		    		if(pid == 0){
						printf("thread + %d\n", getpid());
						if(vOption){
							if(wOption){
								if(execl("./fcounter","fcounter",concatenatePath(path,dirDesc->d_name),"-v","-w","-exit",(char *) 0)<0)
									printf("execle error");
							}
							else
								if(execl("./fcounter","fcounter",concatenatePath(path,dirDesc->d_name),"-v","-exit",(char *) 0)<0)
									printf("execle error");
						}
						else{
							if(wOption){
								if(execl("./fcounter","fcounter",concatenatePath(path,dirDesc->d_name),"-w","-exit",(char *) 0) <0)
									printf("execle error");
							}
							else
								if(execl("./fcounter","fcounter",concatenatePath(path,dirDesc->d_name),"-exit",(char *) 0)<0)
									printf("execle error");
						}
		    		}
		    		else{ //mother thread
		    			childrenAmount++;
		    		}
		    	}
		    }
		    else{
		    	amountOfFiles++;
		    }
		}
		int status;
		if (wOption)
			sleep(15);
		for(i = 0; i< childrenAmount; i++){
			wait(&status);
			childrenAmountOfFiles += WEXITSTATUS(status);
		}
		if (vOption){
			printf("path: %s,"
					"amountOfFiles: %d,\n"
					"children amount of files: %d\n"
					"aggregate amount of my files and my children's amount of files %d\n",
					path,
					amountOfFiles,
					childrenAmountOfFiles,
					amountOfFiles + childrenAmountOfFiles);
		}
	    closedir(dir);
	}
	else
		printf("could not open dir %s\n", path);
	if(exitFromProgram)
		exit(amountOfFiles + childrenAmountOfFiles);
}

int main(int argc, char *argv[]){
	if (argc > 5){
		printf ("Wrong usage: %s [-v] [-w]", argv[1]);
		return -1;
	}
	int i;
	char *path;
	if(argc > 1){
		path = argv[1];
		for (i = 2; i < argc; i++){
			if(cmp("-v",argv[i])){
				vOption = true;
			}
			else if(cmp("-w",argv[i])){
				wOption = true;
			}
			else{
				if(cmp("-exit",argv[i])){
					exitFromProgram = true;
				}else{
					printf ("Wrong usage: %s [-v] [-w]", argv[1]);
					return -1;
				}
			}
		}
	}
	else{
		path = "./";
	}
	seeTree(path);
}
