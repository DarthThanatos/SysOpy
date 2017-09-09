CC = gcc
CFLAGS = -Wall
SRC = airport.c
TARGET = airport
CLEANFLAGS = -f
PROGRAMFLAGS = -pthread -lrt

$(TARGET): $(SRC)
	$(CC) $(USERFLAG) $(CFLAGS) $(SRC) -o $(TARGET) $(PROGRAMFLAGS)

clean:
	rm $(CLEANFLAGS) airport
