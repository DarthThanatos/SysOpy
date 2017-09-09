CC = gcc
CFLAGS = -Wall
SRCC = client.c
SRCS = server.c
SRCP = parent.c
TARGETC = client
TARGETS = server
TARGETP = parent
CLEANFLAGS = -f

run: $(TARGETC) $(TARGETS) $(TARGETP)

$(TARGETC): $(SRCC)
	$(CC) $(CFLAGS) $(SRCC) -o $(TARGETC)


$(TARGETS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGETS)


$(TARGETP): $(SRCP)
	$(CC) $(CFLAGS) $(SRCP) -o $(TARGETP)

clean:
	rm $(CLANFLAGS) $(TARGETC) $(TARGETS) $(TARGETP)

