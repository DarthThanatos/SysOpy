#ifndef COMMON_H_
#define COMMON_H_

#define SPACE_SIZE 4

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr,"%s %s:%d: ",__FILE__, __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \


#endif /* #ifndef COMMON_H_ */