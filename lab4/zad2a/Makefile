CC = gcc
CFLAGS = -Wall
N = 5
CLEANFLAGS= -f

run:
	$(CC) $(CFLAGS) catcher.c -o catcher
	$(CC) $(CFLAGS) sender.c -o sender
	./catcher &
	./sender $(N) &

clean:
	rm $(CLEANFLAGS) sender catcher
