build: clean
	#making static library/archive:
	gcc -c -Wall -pedantic -ansi lista_lab1.c -o lista_lab1_static.o
	ar rcsv libliststat.a lista_lab1_static.o 
	#making shared library:
	gcc -c -Wall -pedantic -ansi -fPIC lista_lab1.c -o lista_lab1_shared.o
	gcc -Wall -pedantic -ansi -shared -fPIC -o liblistshared.so lista_lab1_shared.o	
clean: 
	rm -f *.a
	rm -f *.o
	rm -f *.so
