CC = gcc
CFLAGS = -Wall
SENDER = sender
CATCHER = catcher
SRCSENDER = sender.c
SRCCATCHER = catcher.c
CLEANFLAGS= -f

run: $(SENDER) $(CATCHER)

$(SENDER): $(SRCSENDER)
	$(CC) $(CFLAGS) $^ -o $(SENDER)
$(CATCHER): $(SRCCATCHER)
	$(CC) $(CFLAGS) $^ -o $(CATCHER)

clean:
	rm $(CLEANFLAGS) sender catcher
