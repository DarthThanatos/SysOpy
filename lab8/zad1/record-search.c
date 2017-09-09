#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include <signal.h>
#include <alloca.h>

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
bool *signal_received;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cancelling_section = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t key;

struct record_specific_data{
	char text[1024];
};

struct arguments{
	int index;
	char * wordToSearch;
};

static void unlock(void * unused){
	CHECK(pthread_mutex_unlock(&mutex) == 0);
	CHECK(pthread_mutex_unlock(&cancelling_section) == 0);
	
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

void init(int argc, char *argv[]){
	try (argc != 5, 
	"Wrong usage: ./record-search threads-amount file-with-records-name records-amount word-to-search");
	threads_amount = atoi(argv[1]);
	file_name = argv[2];
	records_amount = atoi(argv[3]);
	word_to_find = argv[4];
	CHECK((private_spaces = malloc(threads_amount * sizeof(char *))) != NULL);
	CHECK((records_descriptor = open(file_name,O_RDONLY,0777)) != -1);
	for (int i = 0; i< threads_amount; i++){
		CHECK((private_spaces[i] = malloc(records_amount * 1024 * sizeof(char))) != NULL);
	}
	CHECK((threads_ids = malloc(threads_amount * sizeof(long unsigned int))) != NULL);
	CHECK((status_array = malloc(threads_amount * sizeof(void *))) != NULL);
	CHECK((signal_received = malloc(threads_amount * sizeof(bool))) != NULL); 
	// ^ a table that tells the thread it can exit because a signal of cancellation has been successfully delivered  
	for (int i = 0; i< threads_amount; i++) signal_received[i] = false;
	#ifdef ASYNCH 
	CHECK(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL) == 0);
	printf("asynchronic cancellation\n");
	#elif SYNCH
	CHECK(pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL) == 0);
	printf("synchronic canellation\n");
	#endif
	CHECK(pthread_key_create(&key, NULL) == 0);
	atexit(clean);
	signal(SIGINT, handler);
}

bool isWordInRecord(struct record_specific_data * record, char *wordToFind){
	int wordLength = strlen(wordToFind);
	for (int i = 1; i < 1024 - wordLength; i++){ 
		// ^ byte at 0 is the record's id, hence we start from i = 1
		bool found = true;
		for (int j = 0; j < wordLength; j++){
			if(record->text[i + j] != wordToFind[j]){
				found = false;
				break;
			}
		}
		if (found) return true;
	}
	return false;
}

void cancelEverybody(){
	printf("cancelling\n");
	for (int j = 0; j < threads_amount; j++){
		if (threads_ids[j] != pthread_self()){
			pthread_cancel(threads_ids[j]);
		}
		signal_received[j] = true;
	}
}

void *thread_browser_function(void *args){
	void * status;
	pthread_cleanup_push(unlock, NULL);
	char *wordToSearch = ((struct arguments *)args)->wordToSearch;
	int myIndex = ((struct arguments *)args)->index;
	int read_bytes;
	bool shouldContinue = true;
	struct record_specific_data *received_records;
	struct record_specific_data *records_array = (struct record_specific_data *)alloca(sizeof(struct record_specific_data) * records_amount);
	while (shouldContinue){

		//critical section
		CHECK(pthread_mutex_lock(&mutex) == 0);
		read_bytes = read(records_descriptor, records_array, records_amount * 1024);
		pthread_setspecific(key, records_array);
		CHECK(pthread_mutex_unlock(&mutex) == 0 );
		//end of critical section

		if (read_bytes == 0){
			status = (void *)END_OF_FILE;
			printf("%d EOF\n", myIndex);
			while(!signal_received[myIndex]);
			break;
		}
		received_records = (struct record_specific_data *) pthread_getspecific(key);
		for (int i = 0; i < records_amount; i++){
			if (isWordInRecord(&received_records[i],wordToSearch)){
				printf("thread %lu word to search %s: %c\n",pthread_self(), (char *)wordToSearch, received_records[i].text[0]);
				shouldContinue = false;
				break;
			}
		}
		if (!shouldContinue){
			status =(void *) FOUND_WORD;
			if(pthread_mutex_trylock(&cancelling_section) == 0 ){
				if(!signalSent){
					signalSent = true;
					#ifndef DETACH
					cancelEverybody();
					// ^ in detached state there is no need to be cancelled
					#endif
				}
				CHECK(pthread_mutex_unlock(&cancelling_section) == 0 );
			}
			while(!signal_received[myIndex]);
		}
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

/*
	cancel_type: deferred, asynchronous
	cancel_state: enable, disable 
	detach_state: detached, joinable
*/

int main(int argc, char* argv[]){
	init(argc, argv);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	#ifdef DETACH
	CHECK(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) == 0);
	for (int i = 0; i<threads_amount; i++) signal_received[i] = true;
	printf("detached state\n");
	#endif
	
	CHECK(pthread_mutex_lock(&mutex) == 0);
	for (int i =0; i<threads_amount; i++){
		struct arguments *arg = malloc(sizeof(struct arguments));
		arg->wordToSearch = word_to_find;
		arg->index = i;
		CHECK(pthread_create(&threads_ids[i], &attr, &thread_browser_function, (void *) arg) == 0);
	}
	CHECK(pthread_mutex_unlock(&mutex) == 0);
	
	#ifdef DETACH
	pthread_exit(0);
	#else
	for (int i =0; i<threads_amount; i++){
		CHECK(pthread_join(threads_ids[i], &status_array[i])== 0); 
	}
	#endif 
}
