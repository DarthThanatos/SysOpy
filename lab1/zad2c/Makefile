SHELL = /bin/bash

useDLLLibrary: clean
	#export LD_LIBRARY_PATH=.
	cp ../zad1/liblistshared.so ./liblistshared.so
	gcc -Wall -ansi -pedantic -o timeTestDLL timeTestDLL.c -ldl
	./timeTestDLL	

clean: 
	rm -f timeTestDLL
	rm -f *.so
