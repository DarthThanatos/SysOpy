CC = gcc
CFLAGS = -Wall
SRCC = client.c
SRCS = server.c
TARGETC = client
TARGETS = server
CLEANFLAGS = -f

run: $(TARGETC) $(TARGETS)

$(TARGETC): $(SRCC)
	$(CC) $(CFLAGS) $(SRCC) -o $(TARGETC)


$(TARGETS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGETS)

clean:
	rm $(CLANFLAGS) $(TARGETC) $(TARGETS)

