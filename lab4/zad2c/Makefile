CC = gcc
CFLAGS = -Wall
N = 3000
CLEANFLAGS= -f

run:
	$(CC) $(CFLAGS) catcher.c -o catcher
	$(CC) $(CFLAGS) sender.c -o sender
	./catcher &
	./sender $(N) &

runSigAction:
	$(CC) $(CFLAGS) catcher_c_sigaction.c -o catcher_c_sigaction
	$(CC) $(CFLAGS) sender_c_sigaction.c -o sender_c_sigaction
	./catcher_c_sigaction &
	./sender_c_sigaction $(N) &

runSignal:
	$(CC) $(CFLAGS) catcher_c_signal.c -o catcher_c_signal
	$(CC) $(CFLAGS) sender_c_signal.c -o sender_c_signal
	./catcher_c_signal &
	./sender_c_signal $(N) &


clean:
	rm $(CLEANFLAGS) sender catcher 
	rm $(CLEANFLAGS) sender_c_sigaction catcher_c_sigaction
	rm $(CLEANFLAGS) sender_c_signal catcher_c_signal
