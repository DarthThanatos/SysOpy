CC = gcc
CFLAGS = -Wall
SRC = philosophers.c
TARGET = philosophers
CLEANFLAGS = -f
PROGRAMFLAGS = -pthread -lrt

$(TARGET): $(SRC)
	$(CC) $(USERFLAG) $(CFLAGS) $(SRC) -o $(TARGET) $(PROGRAMFLAGS)

clean:
	rm $(CLEANFLAGS) philosophers
