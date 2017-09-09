#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

bool cmp(char *firstString, char *secondString){
	int firstLen = strlen(firstString), secondLen = strlen(secondString), i;
	if(firstLen!=secondLen) return false;
	for( i =0; i< firstLen; i++){
		if(firstString[i] != secondString[i])
			return false;
	}
	return true;
}

bool checkPerm(struct stat *fileStat, char * rightsMask){
    char comparator[10]; //rights of file
    comparator[0] = (S_ISDIR(fileStat->st_mode)) ? 'd' : '-';
    comparator[1] = (fileStat->st_mode & S_IRUSR) ? 'r' : '-';
    comparator[2] = (fileStat->st_mode & S_IWUSR) ? 'w' : '-';
    comparator[3] = (fileStat->st_mode & S_IXUSR) ? 'x' : '-';
    comparator[4] = (fileStat->st_mode & S_IRGRP) ? 'r' : '-';
    comparator[5] = (fileStat->st_mode & S_IWGRP) ? 'w' : '-';
    comparator[6] = (fileStat->st_mode & S_IXGRP) ? 'x' : '-';
    comparator[7] = (fileStat->st_mode & S_IROTH) ? 'r' : '-';
    comparator[8] = (fileStat->st_mode & S_IWOTH) ? 'w' : '-';
    comparator[9] = (fileStat->st_mode & S_IXOTH) ? 'x' : '-';
    return cmp(comparator,rightsMask);
}
void printPerm(struct stat *fileStat){
    printf("File Permissions: \t");
    printf( (S_ISDIR(fileStat->st_mode)) ? "d" : "-");
    printf( (fileStat->st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat->st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat->st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat->st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat->st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat->st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat->st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat->st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat->st_mode & S_IXOTH) ? "x" : "-");
    printf("\n\n");
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

void swapStrings(char **firstString, char ** secondString){
    char **addressToSwap = firstString;
    *addressToSwap = *firstString;
    *firstString = *secondString;
    *secondString = *addressToSwap;
}

void listDir(char * path, char *rightsMask, int pathlen){
	DIR *dir;
	struct dirent *dirDesc;
	struct stat statbuf;
	struct tm      *tm;
	char            datestring[256];
	dir = opendir(path);
	if(dir != NULL){
		while((dirDesc = readdir(dir))){
		    int status = lstat(concatenatePath(path,dirDesc->d_name),&statbuf);
		    if(status == -1){
				continue;
		    }
		    if(!S_ISREG(statbuf.st_mode)){
				if(S_ISDIR(statbuf.st_mode) &&
					!cmp(dirDesc->d_name,".") &&
					!cmp(dirDesc->d_name,"..")){
						char *deeperPath = concatenatePath(path,dirDesc->d_name);
						listDir(deeperPath,rightsMask, strlen(deeperPath));
				}
		    }
		    if(S_ISREG(statbuf.st_mode) && checkPerm(&statbuf, rightsMask)){
		        printf("%s ",concatenatePath(path,dirDesc->d_name));
				//printf("ino: %d\n", statbuf.st_ino);
				/* Print size of file. */
				printf(" %9jd", (intmax_t)statbuf.st_size);
				tm = localtime(&statbuf.st_mtime);
				/* Get localized date string. */
				strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);
				printf(" %s\n", datestring);
				printPerm(&statbuf);
		    }
		}
		closedir(dir);
	}
	else
		printf("could not open dir %s", path);
}

int main(int argc, char *argv[]){
	if(argc != 3){
		printf ("Wrong usage!\n");
		return -1;
	}
	char *path = argv[1], *rightsMask = argv[2]; 
	listDir(path, rightsMask, strlen(path));
	return 0;
}
