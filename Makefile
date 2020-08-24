all: obj
	g++ -g -O3 -std=c++11 -pthread -march=native -fpermissive routines.o cryptanalysis.o -o breakSHMWW -lntl -lgmp -lm

obj:
	g++ -g -O3 -std=c++11 -pthread -march=native -fpermissive routines.cpp cryptanalysis.cpp -c -lntl -lgmp -lm

clean:
	rm *.o
