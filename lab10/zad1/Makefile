CC = gcc
CFLAGS = -Wall
SRCC = client.c
SRCS = server.c
TARGETC = client
TARGETS = server
CLEANFLAGS = -f

run: $(TARGETC) $(TARGETS)

$(TARGETC): $(SRCC)
	$(CC) $(CFLAGS) $(SRCC) -o $(TARGETC) -pthread -lrt


$(TARGETS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGETS) -pthread -lrt

clean:
	rm $(CLEANFLAGS) $(TARGETC) $(TARGETS)

