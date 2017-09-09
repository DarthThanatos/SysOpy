CC = gcc
CFLAGS = -Wall
SRCC = client.c
SRCS = server.c
TARGETC = client
TARGETS = server
CLEANFLAGS = -f

run: $(TARGETC) $(TARGETS)

$(TARGETC): $(SRCC)
	$(CC) $(CFLAGS) $(SRCC) -o $(TARGETC) -lrt


$(TARGETS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGETS) -lrt

clean:
	rm $(CLANFLAGS) $(TARGETC) $(TARGETS)

