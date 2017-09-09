LENGTH = 8192
AMOUNT = 1300
FILEPATH = generated.txt

runTests: runGen runSortSys runSortLib	

compileGen:
	gcc -Wall WriteGeneration.c -o WriteGeneration
	
runGen: compileGen
	./WriteGeneration $(FILEPATH) $(LENGTH) $(AMOUNT)

compileSort:
	gcc -Wall sort.c -o sort

runSortLib: compileSort
	./sort $(FILEPATH) $(LENGTH) lib	

runSortSys: compileSort
	./sort $(FILEPATH) $(LENGTH) sys	

measureWriteGen: compileGen
	rm -f file
	(time ./WriteGeneration ./res.txt $(LENGTH) $(AMOUNT)) &> file
	# ^ output time to file 
	echo -e "results of generating records by write:\n$$(cat file)" > file
	# ^ add text at the beginning
	cat file >> wyniki.txt
	# ^ append what is in file to wyniki.txt 
	rm -f file

clean:
	rm -f sort
	rm -f WriteGeneration
	#rm -f *.txt
