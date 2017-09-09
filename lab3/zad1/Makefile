CC=gcc
CFLAGS=-Wall
CLEANFLAG=-f
NVF = 40000
NF = 10000
NFC = 20000
NVC = 30000
N = 50000

runAll: clean runFork runVfork runFclone runVclone

runFork:
	$(CC) $(CFLAGS) forkTest.c -o forkTest
	./forkTest $(N)

runVfork:
	$(CC) $(CFLAGS) vforkTest.c -o vforkTest
	./vforkTest $(N)

runFclone:
	$(CC) $(CFLAGS) fcloneTest.c -o fcloneTest
	./fcloneTest $(N)

runVclone:
	$(CC) $(CFLAGS) vcloneTest.c -o vcloneTest
	./vcloneTest $(N)
	
clean:
	rm $(CLEANFLAG) results/*.txt
	rm $(CLEANFLAG) forkTest
	rm $(CLEANFLAG) vforkTest
	rm $(CLEANFLAG) vcloneTest
	rm $(CLEANFLAG) fcloneTest
