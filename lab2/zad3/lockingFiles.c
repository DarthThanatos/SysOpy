#include <sys/types.h>
#include <sys/stat.h>
#include  <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>


int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len){
    struct flock lock;
    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;
    return(fcntl(fd,cmd,&lock));
}


#define writew_lock(fd, offset, whence, len) \
		lock_reg(fd, F_SETLKW, F_WRLCK,offset, whence, len)

static volatile sig_atomic_t sigflag;
static sigset_t oldmask, zeromask, newmask;

static void sig_usr(int signo){
	sigflag = 1;
	return;
}

void WAIT_CHILD(){
	while (sigflag ==0)
	   sigsuspend(&zeromask);
	sigflag = 0;
	if(sigprocmask(SIG_SETMASK, &oldmask, NULL)<0)
	   printf("SIG_SETMASK error");
}

void WAIT_PARENT(){
	while (sigflag ==0)
	   sigsuspend(&zeromask);
	sigflag = 0;
	if(sigprocmask(SIG_SETMASK, &oldmask, NULL)<0)
	   printf("SIG_SETMASK error");
}

void TELL_PARENT(pid_t pid){
	kill(pid, SIGUSR2);
}

void TELL_CHILD(pid_t pid){
	kill(pid, SIGUSR1);
}


void TELL_WAIT(){
	if(signal(SIGUSR1,sig_usr) == SIG_ERR){
		printf("signal sigurs1 error");
		return;
	}
	if(signal(SIGUSR2,sig_usr) == SIG_ERR){
		printf("signal sigurs2 error");
		return;
	}
	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR2);
	if(sigprocmask(SIG_BLOCK, &newmask, &oldmask)<0){
		printf("sigblock error");
		return;
	}


}

static void lockabyte(const char *name,
		int fd, off_t offset){
	if(writew_lock(fd,offset, SEEK_SET,1)<0){
		printf("%s : writew_lock error\n", name);
		return;
	}
	printf("%s: got the lock, byte %d\n", name, offset);
}

int main(){
	int fd;
	pid_t pid;
	if( (fd = creat("templock", 0600))<0){
		printf("write error");
		return -1;
	}
	if(write(fd, "ab", 2) != 2){
		printf("write error");
		return -1;
	}
	TELL_WAIT();
	if( (pid = fork())<0){
		printf("fork error");
		return -1;
	}
	else if(pid == 0){
		lockabyte("child", fd, 0);
		TELL_PARENT(getppid());
		WAIT_PARENT();
		lockabyte("child", fd, 1);
	}
	else{
		lockabyte("parent", fd, 1);
		TELL_CHILD(pid);
		WAIT_CHILD();
		lockabyte("parent", fd, 0);
	}
	exit(0);
	return 0;
}
