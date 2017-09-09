#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>
#include <time.h>
#include <dlfcn.h>

#define testingElementsAmount 1000
#define fieldsAmount 6

void printTimeDelta(clock_t real, struct tms *startingBenchmark, struct tms *endingBenchmark){
	long clockTackts = sysconf(_SC_CLK_TCK);
	printf("real: %7.2f\n", real / (float) clockTackts);	
	printf("user : %7.2f\n", 
		(endingBenchmark->tms_utime - startingBenchmark->tms_utime)/(double) clockTackts);
	printf("sys op: %7.2f\n",
		(endingBenchmark->tms_stime- startingBenchmark->tms_stime)/ (double) clockTackts);
}

void printTestingElements(char *elements[fieldsAmount]){
	int j;
    	void *Library = dlopen("liblistshared.so",RTLD_LAZY);
    	void (*cout)(char *) = dlsym(Library,"cout");
	for (j = 0; j < fieldsAmount; j++){
	    cout(elements[j]); printf(" ");	
	}
	printf("\n");
}


void testFunction(){
    void *Library = dlopen("liblistshared.so",RTLD_LAZY);
    void (*cout)(char *) = dlsym(Library,"cout");
    void (*addToList)(char *,
		 char *,
		 char *,
		 char *,
		 char *, 
		 char *) = dlsym(Library,"addToList");
    void (*sortList)() = dlsym(Library,"sortList");
    void (*deleteFromList)(char *) = dlsym(Library, "deleteFromList");
    void (*deleteList)() = dlsym(Library, "deleteList");
    void (*printList)() = dlsym(Library, "printList");
    int randomCore, indexOfSearchedEmail;
    int elementMaxLength = 5, i,j,k,stringLength,lettersMode = 1;
    char *elements[fieldsAmount], *emailToFind; 
    struct tms start,adding, sorting, deletingElement, deletingList;
    clock_t startClk, afterAddingClk, afterSortingClk, afterDeletingElementClk, afterDeletingListClk;
    printf("----------------------------------------------------\n");
    randomCore = time(NULL); srand(randomCore);
    if ( (startClk = times(&start)) == -1)
	printf("starting time failed\n");
    indexOfSearchedEmail = rand() % testingElementsAmount;
    for(i = 0; i < testingElementsAmount; i++){
	for (j = 0; j< fieldsAmount; j++){
		stringLength = rand() % elementMaxLength + 1;
		/* ^ so that each string has at least one character*/
		lettersMode = 1; 
		if(j == 2) {
		    stringLength = 8; /*birthdate*/
		    lettersMode = 0;
		}	
		if(j == 3){
		    stringLength = 10;  /*phone number*/
		    lettersMode = 0;
		}

		elements[j] = calloc(stringLength,sizeof (char));
		for (k=0; k<stringLength;k++){
			if(lettersMode)
		            elements[j][k] = (char)(rand() % (122 - 97) + 97);
			else
			    elements[j][k] = (char)(rand() % (57 - 48) + 48);
		}
	}
	if(i == indexOfSearchedEmail){
	    	emailToFind = elements[4];
	}
	(*addToList)(elements[0],
		  elements[3],
		  elements[2],
		  elements[3],
		  elements[4],
		  elements[5]);
	printf("added Element: "); printTestingElements(elements);
    } 
    /*deltatime no. one*/
    if ( (afterAddingClk = times(&adding)) == -1)
	printf("adding time failed\n");
    cout("delta time after adding in reference to start:\n");
    printTimeDelta(afterAddingClk - startClk, &start, &adding);	
    printf("----------------------------------------------------\n");
    printf("List before sorting operation: "); printList();
    sortList();
    printf("List after sorting operation: "); printList();
    /*deltatime no. two*/
    if( (afterSortingClk = times(&sorting)) == -1)
	printf("sorting time failed\n");
    cout("delta time after sorting in reference to start:\n");
    printTimeDelta(afterSortingClk - startClk, &start, &sorting);		
    cout("delta time after sorting in reference to last control point - adding:\n"); 
    printTimeDelta(afterSortingClk - afterAddingClk, &adding, &sorting);
    printf("----------------------------------------------------\n");
    printf("List before deleting: \n"); printList();
    printf("Element to find and delete: "); cout(emailToFind); printf("\n");
    deleteFromList(emailToFind);
    printf("List after deleting: \n"); printList();
    if( (afterDeletingElementClk = times(&deletingElement)) == -1)
	printf("deleting element time failed\n");
    cout("delta time after deleting element in reference to start:\n");
    printTimeDelta(afterDeletingElementClk - startClk, &start, &deletingElement);		
    cout("delta time after deleting element in reference to last control point - sorting:\n"); 
    printTimeDelta(afterDeletingElementClk - afterSortingClk, &adding, &sorting);
    printf("----------------------------------------------------\n");
    deleteList();
    /*deltatime no. four*/
    if((afterDeletingListClk = times(&deletingList)) == -1) 	
	printf("deleting list time failed\n");
    cout("delta time after deleting in reference to start:\n");
    printTimeDelta(afterDeletingListClk - startClk, &start, &deletingList);
    cout("delta time after deleting in reference to last control point: deleting element from list:\n");
    printTimeDelta(afterDeletingListClk - afterDeletingElementClk, &sorting, &deletingElement);
    printf("----------------------------------------------------\n");
}


int main(){
	testFunction();		
	return 0;	
}

