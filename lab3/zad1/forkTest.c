#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>

int  N;
int licznik = 0;
clock_t motherClockStart;
double globalChildrenRealTime = 0;
struct tms motherTmsStart;
double globalChildrenSysTime = 0,globalChildrenUserTime = 0;

void saveRealTime(){
	long clockTackts = sysconf(_SC_CLK_TCK);
	FILE *resultsReal = fopen("results/resultsReal.txt","a");
	clock_t motherClockEnd;
	struct tms motherTmsEnd;
	motherClockEnd = times(&motherTmsEnd);
	double motherRealDelta = (motherClockEnd - motherClockStart)/(double)clockTackts;
	fprintf(resultsReal,"mother fork: %7.2f %d\n",
			motherRealDelta ,N);
	fprintf(resultsReal,"children fork: %7.2f %d\n",
			globalChildrenRealTime,N);
	fprintf(resultsReal,"children + mother fork: %7.2f %d\n",
			(motherRealDelta + globalChildrenRealTime),N);
	fclose(resultsReal);
}

void saveUserAndSysTime(){
	long clockTackts = sysconf(_SC_CLK_TCK);
	FILE *resultsUserAndSys = fopen("results/resultsUserAndSys.txt","a");
	struct tms motherTmsEnd;
	times(&motherTmsEnd);
	double motherUserDelta = (motherTmsEnd.tms_utime - motherTmsStart.tms_utime)/(double)clockTackts;
	double motherSysDelta = (motherTmsEnd.tms_stime - motherTmsStart.tms_stime)/(double)clockTackts;
	fprintf(resultsUserAndSys,"mother fork: %7.2f %d\n",
			motherUserDelta + motherSysDelta ,N);
	fprintf(resultsUserAndSys,"children fork: %7.2f %d\n",
			globalChildrenUserTime + globalChildrenSysTime,N);
	fprintf(resultsUserAndSys,"children + mother fork: %7.2f %d\n",
			globalChildrenUserTime + motherUserDelta +
			globalChildrenSysTime + motherSysDelta,N);
	fclose(resultsUserAndSys);
}

void saveUserTime(){
	long clockTackts = sysconf(_SC_CLK_TCK);
	FILE *resultsUser = fopen("results/resultsUser.txt","a");
	struct tms motherTmsEnd;
	times(&motherTmsEnd);
	double motherUserDelta = (motherTmsEnd.tms_utime - motherTmsStart.tms_utime)/(double)clockTackts;
	fprintf(resultsUser,"mother fork: %7.2f %d\n",
			motherUserDelta,N);
	fprintf(resultsUser,"children fork: %7.2f %d\n",
			globalChildrenUserTime,N);
	fprintf(resultsUser,"children + mother fork: %7.2f %d\n",
			globalChildrenUserTime + motherUserDelta,N);
	fclose(resultsUser);
}

void saveSysTime(){
	long clockTackts = sysconf(_SC_CLK_TCK);
	FILE *resultsSys = fopen("results/resultsSys.txt","a");
	struct tms motherTmsEnd;
	times(&motherTmsEnd);
	double motherSysDelta = (motherTmsEnd.tms_stime - motherTmsStart.tms_stime)/(double)clockTackts;
	fprintf(resultsSys,"mother fork: %7.2f %d\n",
			motherSysDelta,N);
	fprintf(resultsSys,"children fork: %7.2f %d\n",
			globalChildrenSysTime,N);
	fprintf(resultsSys,"children + mother fork: %7.2f %d\n",
			globalChildrenSysTime + motherSysDelta,N);
	fclose(resultsSys);
}

void forkApproach(int argc, char *argv[]){
	long clockTackts = sysconf(_SC_CLK_TCK);
	pid_t pid;
	int i;
	if (argc != 2){
		printf("Wrong usage\n");
		return;
	}
	N = atoi (argv[1]);
	if(N<=0){
		printf("argument cannot be negative\n");
		return;
	}
	motherClockStart = times(&motherTmsStart);
	for(i = 0; i<N; i++) {
		if((pid = fork())< 0 ){
			printf("fork error");
			return;
		}
		else if(pid == 0){
			licznik++;
			_exit(0);
		}
		else{
			int report;
			clock_t childClockStart;
			struct tms childTmsStart;
			childClockStart = times(&childTmsStart);
			wait(&report);
			clock_t childClockEnd;
			struct tms childTmsEnd;
			childClockEnd = times(&childTmsEnd);
			globalChildrenUserTime += (childTmsEnd.tms_utime - childTmsStart.tms_utime)/(double)clockTackts;
			globalChildrenSysTime += (childTmsEnd.tms_stime - childTmsStart.tms_stime)/(double)clockTackts;
			globalChildrenRealTime += (childClockEnd - childClockStart)/(double)clockTackts;
		}
	}
	saveSysTime();
	saveUserTime();
	saveUserAndSysTime();
	saveRealTime();
	printf("counter: %d\n",licznik);
}


int main(int argc, char *argv[]){
	forkApproach(argc, argv);
	return 0;
}
