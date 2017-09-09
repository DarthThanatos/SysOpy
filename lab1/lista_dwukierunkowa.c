#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define N 20

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

typedef struct Node{
    struct Node * next;
    struct Node * prev;
    char name[N];
    char surname[N];
    char birthDate[N];
    char phone[N];
    char email[N];
    char address[N];
} Node;

Node *head = NULL;
Node *tail = NULL;

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

void printListNxt(){
    Node *p; printf("from head to tail: ");
    for(p = head; p!=NULL; p=p->next) {cout(p->email);printf(" ");} printf("\n");
}

void printListBck(){
    Node *p; printf("from tail to head: ");
    for(p = tail; p != NULL; p=p->prev)  {cout(p->email);printf(" ");} printf("\n");
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
        cout("MIN: "); cout(min->email); printf(" "); cout(" i: "); cout(i->email); printf("\n"); printListBck(); printListNxt();
        if(min != i){
            minprev = min->prev;
            inext = i->next;
            if(i == head) {
                printf("i is head\n");
                head = min;
            }
            if(min == tail) {
                printf("min is tail\n");
                tail = i;
            }
            if(i->prev !=NULL) {
                cout("i->prev: "); cout(i->prev->email); printf("\n");
                i->prev->next = min;
            }
            if (min->next!= NULL) {
                cout("min->next: "); cout(min->next->email); printf("\n");
                min->next->prev = i;
            }
            min->prev = i->prev;
            if(min->prev !=NULL) {printf("min->prev: "); cout(min->prev->email); printf("\n");} else printf("min->prev: null");
            i->next = min->next;
            if(i->next !=NULL) {printf("i->next: "); cout(i->next->email); printf("\n");} else printf("i->next: null");
            if(min != inext){
                inext->prev = min;
                if(i->next !=NULL) {printf("i->next: "); cout(i->next->email); printf("\n");} else printf("i->next: null");
                minprev->next = i;
                if(minprev->next !=NULL) {printf("minprev->next: "); cout(minprev->next->email); printf("\n");} else printf("minprev->next: null");
                i->prev = minprev;
                if(i->prev !=NULL) {printf("i->prev: "); cout(i->prev->email); printf("\n");} else printf("i->prev: null");
                min->next = inext;
                if(min->next !=NULL) {printf("min->next: "); cout(min->next->email); printf("\n");} else printf("min->next: null");
            }
            else{
                i->prev = min;
                min->next = i;
                printf("else"); printListNxt();
            }
            i = min;
        }
    }
    printf("out from sort");
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

void test(){
    Node *p;
    initList("rob1","biel","asd","asd","nop","asd");
    addToList("rob1","biel1","asd","asd","fmbi","asd");
    addToList("rob1","biel1","asd","asd","tlx","asd");
    addToList("rob1","biel1","asd","asd","yge","asd");
    addToList("rob1","biel1","asd","asd","ne","asd");
    addToList("rob1","biel1","asd","asd","s","asd");
    addToList("rob1","biel1","asd","asd","cxua","asd");
    addToList("rob1","biel1","asd","asd","ea","asd");
    addToList("rob1","biel1","asd","asd","mqmso","asd");
    addToList("rob1","biel1","asd","asd","qvtd","asd");
    for (p = tail; p!= NULL; p = p->prev) {cout(p->email);printf(" ");} printf("\n");
    sortList();
    printf("\n");
    for (p = tail; p!= NULL; p = p->prev) {cout(p->email);printf(" ");} printf("\n");
    for (p = head; p!= NULL; p=p->next){cout(p->email);printf(" ");} printf("\n");
    deleteFromList("ne");
    for (p = tail; p!= NULL; p = p->prev) {cout(p->email);printf(" ");} printf("\n");
    for (p = head; p!= NULL; p=p->next){cout(p->email);printf(" ");} printf("\n");
}

#define M 6

void testMultiArray(){
    int elementsAmount = 2, i, j;
    char *elements[][M]={
        {"robert","bielas","@","phone","email","address"},
        {"robert1","bielas","@","phone","email","address"}
    };
    for (i = 0; i<elementsAmount; i++){
        for (j = 0; j<M; j++){
            cout(elements[i][j]); printf(" ");
        }
        printf("\n");
    }
}

int main(){
    test();
    return 0;
}

