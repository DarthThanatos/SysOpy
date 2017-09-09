#define N 20 

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

void initList(char *name,
		char *surname,
		char *birthDate,
		char *phone,
		char *email, 
		char *address);

void cout(char *string);

void addToList(char *name,
		char *surname,
		char *birthDate,
		char *phone, 
		char *email, 
		char *address);

void sortList();

void deleteList();

void deleteFromList(char *email);

void printList();

Node *findElement(char *email);

extern Node *head;
extern Node *tail;
