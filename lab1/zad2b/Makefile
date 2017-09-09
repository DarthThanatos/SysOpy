
useSharedDynamicLibrary: clean
	gcc -Wall -ansi -pedantic timeTest.c -L. ../zad1/liblistshared.so -o timeTestShared
	#LD_LIBRARY_PATH=./../zad1
	#export LD_LIBRARY_PATH
	# ^ those two stay here in case shared library has not been found
	./timeTestShared


clean:
	rm -f timeTestShared 
