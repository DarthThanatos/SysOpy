#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "list.h"

void cout(char *string){
    int i;
    for(i =0; string[i] != '\0'; i++) printf("%c",string[i]);
}

int stringLength(char *string){
    int i;
    if (string == NULL) return 0;
    for (i =0; string[i]!= '\0'; i++);
    return i;
}

Node *head = NULL;
Node *tail = NULL;

Node *getHead(){
	return head;
}

Node *getTail(){
	return tail;
}

void printList(){
	Node *p;
	for (p = tail; p!=NULL;p=p->prev) {cout(p->email); printf(" ");} printf("\n");
}

void myStrcpy(char * destination, char *input){
    int i;
    for (i = 0; i<N; i++) destination[i]=input[i];
}

void setNodeData(Node *p ,char *name, char *surname,char *birthDate,char *phone, char *email, char *address){
    myStrcpy(p->name, name);
    myStrcpy(p->surname, surname);
    myStrcpy(p->birthDate, birthDate);
    myStrcpy(p->phone, phone);
    myStrcpy(p->email, email);
    myStrcpy(p->address, address);
}

void initList(char *name, char *surname,char *birthDate,char *phone, char *email, char *address){
    if(head == NULL){
        head = malloc(sizeof (Node));
        setNodeData(head,name,surname,birthDate,phone,email,address);
        head->prev = NULL;
        head->next = NULL;
        tail = head;
    }
}

void deleteList(){
    Node *p = head;
    while(p!=NULL){
        head = head->next;
        free(p);
        p = head;
    }
    tail = NULL;
}

void addToList(char *name, char *surname,char *birthDate,char *phone, char *email, char *address){
    if(head == NULL) initList(name,surname,birthDate,phone,email,address);
    else {
        Node *p = malloc(sizeof (Node));
        tail->next = p;
        p->prev = tail;
        p->next = NULL;
        tail = p;
        setNodeData(p,name,surname,birthDate,phone,email,address);
    }
}

void swapStrings(char **firstString, char ** secondString){
    char **addressToSwap = firstString;
    *addressToSwap = *firstString;
    *firstString = *secondString;
    *secondString = *addressToSwap;
}

bool areStringsEqual(char *firstString, char *secondString){
    int i;
    if(firstString==NULL || secondString == NULL || stringLength(firstString) != stringLength(secondString)) return false;
    if(stringLength(firstString) < stringLength(secondString)){
        swapStrings(&firstString,&secondString);
    }
    for(i = 0; firstString[i]!= '\0'; i++){
        if(firstString[i]!= secondString[i])
            return false;
        }
    return true;
}

Node *findElement(char *email){
    Node *p;
    for(p = head; p!=NULL && !areStringsEqual(p->email,email); p = p->next);
    return p;
}

bool firstStringBigger(char *firstString, char * secondString){
    int i = 0;
    if(firstString == NULL) return false;
    if(secondString == NULL) return true;
    if(stringLength(firstString)<stringLength(secondString)) return false;
    if(stringLength(firstString)>stringLength(secondString)) return true;
    if(areStringsEqual(firstString,secondString)) return false;

    while(firstString[i] == secondString[i]) i++;
    if((int)firstString[i] < (int)secondString[i])
        return false;
    return true;
}

void sortList(){
    Node *i, *j, *min;
    Node *inext,* minprev;
    for (i = head; i!=NULL; i=i->next){
        min = i;
        for (j = i->next; j!=NULL; j = j->next){
            if(firstStringBigger(min->email,j->email))
                min = j;
        }
        if(min != i){
            minprev = min->prev;
            inext = i->next;
            if(i == head) {
                head = min;
            }
            if(min == tail) {
                tail = i;
            }
            if(i->prev !=NULL) {
                i->prev->next = min;
            }
            if (min->next!= NULL) {
                min->next->prev = i;
            }
            min->prev = i->prev;
            i->next = min->next;
            if(min != inext){
                inext->prev = min;
                minprev->next = i;
                i->prev = minprev;
                min->next = inext;
            }
            else{
                i->prev = min;
                min->next = i;
	    }
            i = min;
        }
    }
}

void deleteFromList(char *email){
    Node *p;
    if(head == NULL) return;
    p = findElement(email);
    if(p == head) {
        head = head->next;
        free(p);
        if(head!=NULL)head->prev = NULL;
        else tail = NULL;
    }
    else
        if(p == tail) {
            tail = head->prev;
            free(p);
            if(tail!=NULL)tail->next = NULL;
            else head = NULL;
        }
        else{/*middle element*/
            p->prev->next = p->next;
            p->next->prev = p->prev;
            free(p);
        }
}



