CC = gcc
CFLAGS = -Wall
SRC = record-search.c
TARGET = record-search
CLEANFLAGS = -f
PROGRAMFLAGS = -pthread -lrt

$(TARGET): $(SRC)
	$(CC) $(USERFLAG) $(CFLAGS) $(SRC) -o $(TARGET) $(PROGRAMFLAGS)

clean:
	rm $(CLEANFLAGS) record-search
