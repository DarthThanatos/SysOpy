CC = gcc
CFLAGS = -Wall
EXE = pipeTrFold
SRC = pipeTrFold.c
CLEANFLAGS= -f

run: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $^ -o $(EXE)

clean:
	rm $(CLEANFLAGS) pipeTrFold
