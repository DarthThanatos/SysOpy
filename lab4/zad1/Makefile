CC = gcc
CFLAGS = -Wall
N = 3
TEXT = baldur
RMFLAG = -f

run:
	$(CC) $(CFLAGS) printer.c -o printer
	./printer $(TEXT) $(N) &

clean:
	rm $(RMFLAG) printer 
