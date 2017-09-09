#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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
#include <stdbool.h>

char * rightsMask;

bool cmp(char *firstString, char *secondString){
	int firstLen = strlen(firstString), secondLen = strlen(secondString), i;
	if(firstLen!=secondLen) return false;
	for( i =0; i< firstLen; i++){
		if(firstString[i] != secondString[i])
			return false;
	}
	return true;
}

bool checkPerm(const struct stat *fileStat){
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
    comparator[10] = '\0';
    return cmp(comparator,rightsMask);
}
void printPerm(const struct stat *fileStat){
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

static int display_info( const char *path, const struct stat *statbuf, int tflag, struct FTW *ftwbuf){
	struct tm *tm;
	char datestring[256];
	if(tflag == FTW_F){
	    if(checkPerm(statbuf)){
      		printf("\n%s\n", realpath(path,NULL));
		/* Print size of file. */
		printf("size:%9jd\n", (intmax_t)statbuf->st_size);
		tm = localtime(&(statbuf->st_mtime));
		/* Get localized date string. */
		strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);
		printf("last used:%s\n", datestring);
		printPerm(statbuf);
	    }
	}
        return 0;           /* To tell nftw() to continue */
}

int main(int argc, char *argv[]){

    if(argc != 3){
	printf ("Wrong usage!\n");
	return -1;
    }
    char *path = argv[1];
    rightsMask= argv[2];
    int flags = 0;
    flags |= FTW_PHYS; //do not follow symbol link

    if (nftw(path, display_info, 20, flags)== -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
