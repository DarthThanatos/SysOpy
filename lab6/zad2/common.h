#ifndef COMMON_H_
#define COMMON_H_
#define MAX_QUEUE_SIZE 20
#define MAX_MSGS 100

struct initReport{
	char a;
	//int mtype;
	char queue_id[21];
};

struct serverBack{
	int mtype;
	int num;
	int id;
	char mText[128];
};

//-----------------------------------

struct secondPhase_ClientWantsNumber{
	char a;
	int id;
};

struct secondPhase_ServerGivesNumber{
	long mType;
	int num;
};

struct secondPhase_ClientGivesNumber{
	char a;
	long mType;
	int num;
	int id;
	int isPrime;
};

struct clientExits{
	char a;
	int id;
};

#define CLIENT_MSG_TYPE 2
#define SERVER_MSG_TYPE 1

#define CLIENT_READY 3
#define CLIENT_NOT_READY 4

#define SERVER_GIVES_DISPATCH 5
#define CLIENTS_AMOUNT 3

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \


#endif /* #ifndef COMMON_H_ */