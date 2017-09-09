#ifndef COMMON_H_
#define COMMON_H_
#include <semaphore.h>
#include <errno.h>

#define SPACE_SIZE 4
#define SPACE_NAME "common_space"
#define READER_SEMAPHORE "/reader_sem"
#define WRITER_SEMAPHORE "/writer_sem"
#define READERS_IN_LIBRARY 5

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s %s:%d: ", __FILE__, __func__, __LINE__); \
            perror(#x); \
			printf("%d %d\n", errno, EINVAL);\
            exit(-1); \
        } \
    } while (0) \


#endif /* #ifndef COMMON_H_ */