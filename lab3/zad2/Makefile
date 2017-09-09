CC = gcc
CFLAGS = -Wall
RMFLAGS = -f
PROGFLAGS = -w

runPure:
	$(CC) $(CFLAGS) fcounter.c -o fcounter
	./fcounter ./ $(PROGFLAGS)
runVar:
	$(CC) $(CFLAGS) fcounterVar.c -o fcounterVar
	./fcounterVar $(PROGFLAGS) 

clean:
	rm $(RMFLAGS) fcounter
	rm $(RMFLAGS) fcounterVar
