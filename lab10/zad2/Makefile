CC = gcc
CFLAGS = -Wall
SRCC = client.c
SRCS = server.c
TARGETC = client
TARGETS = server
CLEANFLAGS = -f

run: $(TARGETC) $(TARGETS) 

$(TARGETC): $(SRCC)
	$(CC) $(CFLAGS) $(SRCC) -o $(TARGETC) -lrt -pthread


$(TARGETS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGETS) -lrt -pthread

clean:
	rm $(CLEANFLAGS) $(TARGETC) $(TARGETS) 

