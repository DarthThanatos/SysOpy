CC = gcc
CFLAGS = -Wall
RDFIFO = rdfifo
WRFIFO = wrfifo
SRCRDFIFO = rdfifo.c
SRCWRFIFO = wrfifo.c
CLEANFLAGS= -f

run: $(RDFIFO) $(WRFIFO)

$(RDFIFO): $(SRCRDFIFO)
	$(CC) $(CFLAGS) $^ -o $(RDFIFO)
$(WRFIFO): $(SRCWRFIFO)
	$(CC) $(CFLAGS) $^ -o $(WRFIFO)

clean:
	rm $(CLEANFLAGS) $(RDFIFO) $(WRFIFO) queue
