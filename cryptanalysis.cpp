#include "routines.hpp"
#include <time.h>

#define VERBOSE

using namespace std;
using namespace NTL;

param_set getParameterSet(int instance) {
	param_set P;
	if (instance==1) {
		P.n=4096;
		P.k=539;
		P.dGV=1191;
		P.l=4;
		P.nprime=1024;
		P.kprime=890;
		P.w1=31;
		P.w2=531;
		P.secu=80;
		#ifdef VERBOSE
			printf("[getParameterSet] Parameter set PARA-1 selected\n");
			printf("[getParameterSet] P.n=4096;\n");
			printf("[getParameterSet] P.k=539;\n");
			printf("[getParameterSet] P.dGV=1191;\n");
			printf("[getParameterSet] P.l=4;\n");
			printf("[getParameterSet] P.nprime=1024;\n");
			printf("[getParameterSet] P.kprime=890;\n");
			printf("[getParameterSet] P.w1=31;\n");
			printf("[getParameterSet] P.w2=531;\n");
			printf("[getParameterSet] P.secu=80;\n");
		#endif
	}
	else if (instance==2) {
		P.n=8192;
		P.k=1065;
		P.dGV=2383;
		P.l=8;
		P.nprime=1024;
		P.kprime=880;
		P.w1=53;
		P.w2=807;
		P.secu=128;
		#ifdef VERBOSE
			printf("[getParameterSet] Parameter set PARA-2 selected\n");
			printf("[getParameterSet] P.n=8192;\n");
			printf("[getParameterSet] P.k=1065;\n");
			printf("[getParameterSet] P.dGV=2383;\n");
			printf("[getParameterSet] P.l=8;\n");
			printf("[getParameterSet] P.nprime=1024;\n");
			printf("[getParameterSet] P.kprime=880;\n");
			printf("[getParameterSet] P.w1=53;\n");
			printf("[getParameterSet] P.w2=807;\n");
			printf("[getParameterSet] P.secu=128;\n");
		#endif
	} else {
		printf("[getParameterSet] Parameter set undefined, should be either 1 (80 bits secu) or 2 (128 bits secu), see SHMWW.\n");
		P.n = P.k = P.nprime = P.kprime = P.l = P.w1 = P.w2 = P.dGV = P.secu = 0;
	}
	return P;
}

uint64_t setThreshold(param_set P, int number_of_signatures) {
	return uint64_t(number_of_signatures * ((double(P.w1) / double(P.kprime)) + (double(P.w2) / double(P.n)) * (1 - 2*double(P.w1) / double(P.kprime))));
}


void computeSignatureWeightDistribution(int nb_of_signatures, char* filename, uint64_t* weights, param_set P) {
	FILE *f;
	if ((f=fopen(filename, "r"))==NULL)	{
		fprintf(stdout, "Error opening file %s in read mode...\n", filename);
		exit(-1);
	}
	//signature S;
	vec_GF2 z;
	z.SetLength(P.n);
	for (int i = 0; i < nb_of_signatures; ++i) {
		readVectFromFile(f, z, P.n);
		for(int j=0 ; j<P.n ; j++) {
			if (z[j] == 1)
				weights[j]++;
		}
	}
	fclose(f);
}

void readPK(char* filename, mat_GF2 &H, mat_GF2 &S, param_set P) {
	FILE *f;
	if ((f=fopen(filename, "r"))==NULL)	{
		fprintf(stdout, "Error opening file %s in read mode...\n", filename);
		exit(-1);
	}
	readMatFromFile(f, H, P.n-P.k, P.n);
	readMatFromFile(f, S, P.n-P.k, P.kprime);
	fclose(f);
}

void readSK(char* filename, mat_GF2 &E, param_set P) {
	FILE *f;
	if ((f=fopen(filename, "r"))==NULL)	{
		fprintf(stdout, "Error opening file %s in read mode...\n", filename);
		exit(-1);
	}
	readMatFromFile(f, E, P.kprime, P.n);
	fclose(f);
}

void genSubH(mat_GF2 &subH, mat_GF2 H, uint16_t* random_indices, param_set P) {
	int nimp;
	for (int i = 0; i < P.n-P.k; ++i) {		
		for (int j = 0; j < P.n-P.k; ++j) {
			subH[i][j] = GF2(H[i][random_indices[j]]);
		}
	}
}

void ISDtry(vec_GF2 &Ei, mat_GF2 H, mat_GF2 S, int line, uint16_t *random_indices, param_set P) {
	clear(Ei);
	mat_GF2 subH;
	subH.SetDims(P.n-P.k, P.n-P.k);
	genSubH(subH, H, random_indices, P);

	vec_GF2 subS;
	subS.SetLength(P.n-P.k);
	for (int i = 0; i < P.n-P.k; ++i)
		subS[i] = S[i][line];

	GF2 det;
	vec_GF2 solution;
	solution.SetLength(P.n-P.k);
	solve(det, subH, solution, subS);
	for (int i = 0; i < P.n-P.k; ++i) {
		if (solution[i]==1) {
			Ei[random_indices[i]] = 1;
		}
	}
}

int random_columnsTOrandom_indices(uint16_t *random_indices, vec_GF2 random_columns, param_set P) {
	int nbColumns=0;
	for (int i = 0; i < P.n; ++i)	{
		if (IsOne(random_columns[i])) {
			random_indices[nbColumns++] = i;
		}
	}
	return nbColumns;
}

void recoverE(mat_GF2 &guessedSK, mat_GF2 H, mat_GF2 S, param_set P, vec_GF2 random_columns){
	uint64_t attemps=0;
	uint64_t maxWeight = P.l*(P.nprime-P.kprime);
	uint16_t random_indices[P.n-P.k] = {0};
	int nbColumns = random_columnsTOrandom_indices(random_indices, random_columns, P);
	for (int i = 0; i < P.kprime; ++i) {
		bool notFinished = true;
		while (notFinished) {
			int startIndex = nbColumns;
			while (startIndex < P.n-P.k) {
				uint16_t random_index = rand()%P.n;
				bool indexExists=false;
				for (int j = 0; j < startIndex; ++j) {
					if (random_index == random_indices[j]) {
						indexExists=true;
						break;
					}
				}
				if (!indexExists)
					random_indices[startIndex++]=random_index;
			}

			ISDtry(guessedSK[i], H, S, i, random_indices, P);

			if (Hweight(guessedSK[i], P.n)!=0 && Hweight(guessedSK[i], P.n)<=maxWeight) {
				notFinished=false;
				attemps=0;
				printf("Number of lines recovered in guessedSK: %d/%d\n", i+1, P.kprime);
			}
		}
	}
}

int main(int argc, char const *argv[]) {
	if (argc!=3)
	{
		cerr << "USAGE: ./breakSHMWW PARAMETER_SET TARGET_NUMBER_OF_SIGNATURE\n";
		exit(-1);
	}

	srand(time(NULL));

	param_set P = getParameterSet(atoi(argv[1]));
	int number_of_signatures = atoi(argv[2]);

	//load the public key
	mat_GF2 H, S;
	H.SetDims(P.n-P.k, P.n);
	S.SetDims(P.n-P.k, P.kprime);

	char rootFilename[64] = "samples/PARA-";
	strcat(rootFilename, argv[1]);

	// load the public key H, S
	char tmpFilename[64];
	strcpy(tmpFilename, rootFilename);
	strcat(tmpFilename, "pk");
	#ifdef VERBOSE
	cout << "Reading pk in file \"" << tmpFilename << "\"" << endl;
	#endif
	readPK(tmpFilename, H, S, P);

	// load the signatures and compute their sum
	strcpy(tmpFilename, rootFilename);
	strcat(tmpFilename, "sigs");
	uint64_t weights[P.n] = {0};
	#ifdef VERBOSE
	cout << "Reading signatures in file \"" << tmpFilename << "\"" << endl;
	#endif
	computeSignatureWeightDistribution(number_of_signatures, tmpFilename, weights, P);

	// initializing threshold
	uint64_t threshold = setThreshold(P, number_of_signatures);

//	threshold = atoi(argv[3]); //////TMP FIX !!!

	#ifdef VERBOSE
	printf("Threshold = %" PRIu64 "\n", threshold);
	#endif

	// guess the random columns
	int cpt=0;
	vec_GF2 random_columns;
	random_columns.SetLength(P.n);

	for (int i = 0; i < P.n; ++i) {
		if (weights[i] > threshold) {
			random_columns[i] = 1;
			cpt++;
		}
	}

//	printf("cpt=%d\n", cpt);

	//ISD to recover the lines of E
	mat_GF2 guessedSK, realSK;
	guessedSK.SetDims(P.kprime, P.n);
	recoverE(guessedSK, H, S, P, random_columns);

	//load the real secret key for testing the attack
	strcpy(tmpFilename, rootFilename);
	strcat(tmpFilename, "sk");	
	#ifdef VERBOSE
	cout << "Reading sk in file \"" << tmpFilename << "\"" << endl;
	#endif
	realSK.SetDims(P.kprime, P.n);
	readSK(tmpFilename, realSK, P);

	bool secretKeyRecovered=true;
	for (int i = 0; i < P.kprime; ++i) {
		for (int j = 0; j < P.n; ++j) {
			if (guessedSK[i][j] != realSK[i][j]) {
				secretKeyRecovered=false;
				break;
			}
		}
	}
	if (secretKeyRecovered)	{
		printf("Secret key correctly recovered \\o/\n");
	} else {
		printf("FAIL :(\n");
	}

	return 0;
}
