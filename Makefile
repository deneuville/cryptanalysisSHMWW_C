all: obj
	g++ -I/usr/local/include -L/usr/local/lib -Wall -g -O3 -std=c++11 -pthread -march=native -fpermissive routines.o cryptanalysisSHMWW.o -o breakSHMWW -lntl -lgmp -lgf2x -lm

obj:
	g++ -I/usr/local/include -L/usr/local/lib -Wall -g -O3 -std=c++11 -pthread -march=native -fpermissive routines.cpp cryptanalysisSHMWW.cpp -c -lntl -lgmp -lgf2x -lm

clean:
	rm -rf *.o breakSHMWW

