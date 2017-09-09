#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

char *cmd1[] = { "tr", "'[:lower:]'", "'[:upper:]'", 0 };
char *cmd2[] = { "fold", "-w", "0", 0};
void runTr(int pfd[]);
void runFold(int pfd[]);

void init(int argc, char *argv[]){
	if (argc !=2) {
		printf("Bad usage!\n");
		exit(1);
	}
	cmd2[2] = argv[1];
}

int main(int argc, char **argv){
	init(argc, argv);
	int pid, status;
	int fd[2];
	pipe(fd);
	runTr(fd);
	runFold(fd);
	close(fd[0]); close(fd[1]); 	/* close both file descriptors on the pipe */
	while ((pid = wait(&status)) != -1);	/* pick up all the dead children */
	exit(0);
}


void runTr(int pfd[])	/* run the first part of the pipeline, cmd1 */
{
	int pid;
	switch (pid = fork()) {
		case 0: /* child */
			dup2(pfd[1], 1);	/* this end of the pipe becomes the standard output */
			close(pfd[0]); 		/* this process don't need the other end */
			execvp(cmd1[0], cmd1);	/* run the command */
			perror(cmd1[0]);	/* it failed! */

		default: /* parent does nothing */
			break;

		case -1:
			perror("fork");
			exit(1);
		}
}

void runFold(int pfd[])	/* run the second part of the pipeline, cmd2 */
{
	int pid;
	switch (pid = fork()) {
		case 0: /* child */
			dup2(pfd[0], 0);	/* this end of the pipe becomes the standard input */
			close(pfd[1]);		/* this process doesn't need the other end */
			execvp(cmd2[0], cmd2);	/* run the command */
			perror(cmd2[0]);	/* it failed! */

		default: /* parent does nothing */
			break;

		case -1:
			perror("fork");
			exit(1);
		}
}
