#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <unistd.h>
#include <time.h>

#ifdef VCLONE
#include <sched.h>
#endif
#ifdef FCLONE
#include <sched.h>
#endif

long licznik;

long child_time, proc_time;

int fun()
{
	licznik++;
	long t = times(NULL);
	_exit(t - proc_time);
}

int main(int argc, char ** argv)
{
	child_time = 0;
	struct tms tt;
	long int clk = sysconf(_SC_CLK_TCK);
	long start_point = times(NULL);

	if(argc < 2)
	{
		printf("usage: %s N\n", argv[0]);
		exit(1);
	}

	int n = atoi(argv[1]);

	int i = 0;
	#ifdef FCLONE
	void * child_stack = (void *)malloc(16384);
	child_stack += 16384;
	#elif VCLONE
	void * child_stack = (void *)malloc(16384);
	child_stack += 16384;
	#endif
	for(i = 0 ; i < n ; i++)
	{
		proc_time = times(NULL);

		#ifdef FORK
		pid_t PID = fork();
		#elif VFORK
		pid_t PID = vfork();
		#elif FCLONE
		int (*fn)(void *) = fun;
		pid_t PID = clone(fn, child_stack, SIGCHLD, NULL);
		#elif VCLONE
		int (*fn)(void *) = fun;
		pid_t PID = clone(fn, child_stack, SIGCHLD | CLONE_VM | CLONE_VFORK | CLONE_FS | CLONE_FILES, NULL);
		#endif

		if(PID < 0)
		{
			fprintf(stderr, "Blad fork()\n");
			exit(-1);
		}
		else
		if(PID == 0)
		{
			fun();
		}
		else
		{
			int status = -1;
			wait(&status);
			child_time += WEXITSTATUS(status);
		}
	}

	printf("licznik = %ld\n", licznik);

	long t;
	t = times(&tt);
	proc_time = t - start_point;

	float clkf = (float)clk;

	printf("\n..:: PARENT ::..\n");
	printf("REAL\t\tUSER\t\tSYS\n");
	printf("%.3f\t\t%.3f\t\t%.3f\n", proc_time / clkf, tt.tms_utime / clkf, tt.tms_stime / clkf);
	
	printf("\n..:: CHILDREN ::..\n");
	printf("REAL\t\tUSER\t\tSYS\n");
	printf("%.3f\t\t%.3f\t\t%.3f\n", child_time / clkf, tt.tms_cutime / clkf, tt.tms_cstime / clkf);

	return 0;
}
