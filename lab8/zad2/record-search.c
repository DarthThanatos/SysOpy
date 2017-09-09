#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include <signal.h>

#define MIN_WORKING_ID 100000000000000
#define FOUND_WORD 0
#define END_OF_FILE 1

int threads_amount;
char *file_name;
int records_amount;
char *word_to_find;
char **private_spaces;
int records_descriptor;
pthread_t *threads_ids;
void **status_array;
bool signalSent = false;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void unlock(void * unused){
	CHECK(pthread_mutex_unlock(&mutex) == 0);
}

void handler(int signo){
	printf("caught\n");
	exit(0);
}

void clean(){
	printf("\n\nexiting...\n\n");
	CHECK(close(records_descriptor) != -1);
	CHECK(pthread_mutex_destroy(&mutex) == 0);
}

#ifdef DIV_BY_ZERO
void *divisionByZero(void * unused){
	printf("%lu generating division by zero excetion\n",pthread_self());
	int val = 1/0;
	return NULL;
}
#endif

void init(int argc, char *argv[]){
	//try (argc != 5, 
	//"Wrong usage: ./record-search threads-amount file-with-records-name records-amount word-to-search");
	threads_amount = atoi(argv[1]);
	file_name = argv[2];
	records_amount = atoi(argv[3]);
	word_to_find = argv[4];
	CHECK((private_spaces = malloc(threads_amount * sizeof(char *))) != NULL);
	CHECK((records_descriptor = open(file_name,O_RDONLY,0777)) != -1);
	for (int i = 0; i< threads_amount; i++){
		CHECK((private_spaces[i] = malloc(records_amount * 1024 * sizeof(char))) != NULL);
	}
	CHECK((threads_ids = malloc(threads_amount * sizeof(long unsigned int))));
	CHECK((status_array = malloc(threads_amount * sizeof(void *))) != NULL);
	
	#ifdef ASYNCH 
	CHECK(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL) == 0);
	printf("asynchronic cancellation\n");
	#elif SYNCH
	CHECK(pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL) == 0);
	printf("synchronic canellation\n");
	#endif
	atexit(clean);
	signal(SIGINT, handler);
	#ifdef MAIN_BLOCKS
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGKILL);
	sigaddset(&set, SIGSTOP);
	pthread_sigmask(SIG_SETMASK, &set, NULL);
	#endif
}

bool isWordInRecord(char record[1024], char *wordToFind){
	int wordLength = strlen(wordToFind);
	for (int i = 0; i < 1024 - wordLength; i++){
		bool found = true;
		for (int j = 0; j < wordLength; j++){
			if(record[i + j] != wordToFind[j]){
				found = false;
				break;
			}
		}
		if (found) return true;
	}
	return false;
}


void cancelEverybody(){
	for (int j = 0; j < threads_amount; j++){
		if (threads_ids[j] != pthread_self() ){
			#ifdef DETACH
			if(threads_ids[j] > MIN_WORKING_ID)
				pthread_cancel(threads_ids[j]);
			#else
			while(threads_ids[j] < MIN_WORKING_ID);
			pthread_cancel(threads_ids[j]);
			#endif
		}
	}
}

void thread_handle(int signo){
	printf("Got %d, pid: %d, tid: %lu\n", signo, getpid(), pthread_self());
}

void *thread_browser_function(void *wordToSearch){
	void * status;
	pthread_cleanup_push(unlock, NULL);
	#if defined(THREAD_UNSTANDARD_HANDLE) || defined(THREAD_HANDLE_MAIN_SENDS)
	signal(SIGUSR1, thread_handle);
	signal(SIGSTOP, thread_handle);
	signal(SIGTERM, thread_handle);
	signal(SIGKILL, thread_handle);
	#endif
	#ifdef THREAD_BLOCKS
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGKILL);
	sigaddset(&set, SIGSTOP);
	pthread_sigmask(SIG_SETMASK, &set, NULL);
	#endif
	#ifdef SLEEP
	printf("thread %lu sleeping\n",pthread_self());
	sleep(50);
	#endif
	int read_bytes;
	bool shouldContinue = true;
	while (shouldContinue){
		char *record = malloc(records_amount * 1024 + 1);
		record[records_amount * 1024] = '\0';
		char *singleRecord = malloc (1025);
		singleRecord[1024] = '\0';
		CHECK(pthread_mutex_lock(&mutex) == 0);
		read_bytes = read(records_descriptor, record, records_amount * 1024);
		CHECK(pthread_mutex_unlock(&mutex) == 0 );
		if (read_bytes == 0){
			status = (void *)END_OF_FILE;
			break;
		}
		for (int i = 0; i < records_amount; i++){
			strncpy(singleRecord,record + i * 1024,1024);
			if (isWordInRecord(singleRecord,wordToSearch)){
				printf("thread %lu word to search %s: %c\n",pthread_self(), (char *)wordToSearch, singleRecord[0]);
				shouldContinue = false;
				break;
			}
		}
		if (!shouldContinue && !signalSent){
			cancelEverybody();
			signalSent = true;
			status =(void *) FOUND_WORD;
		}
		free(singleRecord);
		free(record);
		#ifdef SYNCH
		pthread_testcancel();
		#endif
	}
	#ifdef DETACH
	pthread_testcancel();
	#endif
	pthread_cleanup_pop(false);
	return status; 
}

int main(int argc, char* argv[]){
	init(argc, argv);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	#ifdef DETACH
	CHECK(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) == 0);
	printf("detached state\n");
	#endif
	#ifdef THREAD_UNSTANDARD_HANDLE
	printf("main thread: %lu\n", pthread_self());
	#endif
	for (int i =0; i<threads_amount; i++){
		CHECK(pthread_create(&threads_ids[i], &attr, &thread_browser_function, (void *) word_to_find) == 0);
	}
	#ifdef DIV_BY_ZERO
	pthread_t pest;
	CHECK(pthread_create(&pest, &attr, &divisionByZero, NULL) == 0);
	#endif
	#if defined(THREAD_BLOCKS) || defined(THREAD_HANDLE_MAIN_SENDS)
	sleep(1);
	printf("mother sent signal\n");
	pthread_kill(threads_ids[threads_amount-1], atoi(argv[5]));
	#endif
	#ifdef DETACH
	pthread_exit(0);
	#else
	for (int i =0; i<threads_amount; i++){
		CHECK(pthread_join(threads_ids[i], &status_array[i])== 0); 
		printf("status of %lu thread: %d\n", threads_ids[i], (int)status_array[i]);
	}
	#endif 
}
