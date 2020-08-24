#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

#include <NTL/tools.h>
#include <NTL/GF2.h>
#include <NTL/vec_GF2.h>
#include <NTL/mat_GF2.h>

#include <chrono>

using namespace std;
using namespace NTL;


typedef struct {
	uint8_t* z, c, commitment;	
}signature;

typedef struct {
	int n;
	int k;
	int dGV;
	int l;
	int nprime;
	int kprime;
	int w1;
	int w2;
	int secu;
}param_set;

uint64_t Hweight(vec_GF2 vect, int length);

void printVect(vec_GF2 vect, int length);

void printMat(mat_GF2 M, int nbRows, int nbColumns);

void readVectFromFile(FILE* f, vec_GF2 &v, int length);

void readMatFromFile(FILE* f, mat_GF2 &M, int nbRows, int nbColumns);

