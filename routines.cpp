#include "routines.hpp"

//#define ROUT_VERBOSE

using namespace NTL;

uint64_t Hweight(vec_GF2 vect, int length) {
	uint64_t weight=0;
	for (int i = 0; i < length; ++i) {
		if (IsOne(vect[i])) {
			++weight;
		}
	}
	return weight;
}

void printVect(vec_GF2 vect, int length) {
	cout << "[";
	for (int i = 0; i < length-1; ++i)
		cout << vect[i] << " ";
	cout << vect[length-1] << endl;
}

void printMat(mat_GF2 M, int nbRows, int nbColumns) {
	cout << "[\n";
	for (int i = 0; i < nbRows; ++i)
		printVect(M[i], nbColumns);
	cout << "]\n";
}

void readVectFromFile(FILE* f, vec_GF2 &v, int length) {
	int ret, val;
	for (int i = 0; i < length; ++i) {
		ret = fscanf(f, "%d ", &val);
		v[i] = val;
		#ifdef ROUT_VERBOSE
		if (ret != 1)
			cerr << "v[" << i << "] --> " << ret << "\n";
		#endif
	}
	//printZ(v, length);
}

void readMatFromFile(FILE* f, mat_GF2 &M, int nbRows, int nbColumns) {
	for (int i = 0; i < nbRows; ++i) {
		readVectFromFile(f, M[i], nbColumns);
	}
}

