CC = gcc
CFLAGS = -Wall
EXE = popenText
SRC = popenText.c
CLEANFLAGS= -f

run: $(EXE) 

$(EXR): $(SRC)
	$(CC) $(CFLAGS) $^ -o $(EXE)

clean:
	rm $(CLEANFLAGS) $(EXE)
