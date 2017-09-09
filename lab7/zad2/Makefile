CC = gcc
CFLAGS = -Wall
SRCR = reader.c
SRCW = writer.c
SRCP = parent.c
TARGETR = reader
TARGETW = writer
TARGETP = parent
CLEANFLAGS = -f

run: $(TARGETR) $(TARGETW) $(TARGETP)

$(TARGETR): $(SRCR)
	$(CC) $(CFLAGS) $(SRCR) -o $(TARGETR)


$(TARGETW): $(SRCW)
	$(CC) $(CFLAGS) $(SRCW) -o $(TARGETW)


$(TARGETP): $(SRCP)
	$(CC) $(CFLAGS) $(SRCP) -o $(TARGETP)

clean:
	rm $(CLANFLAGS) $(TARGETR) $(TARGETW) $(TARGETP)

