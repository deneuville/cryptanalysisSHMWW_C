#include "routines.hpp"

#include <chrono>

#define _TOTAL_INFO_ 1
#define _SHMWW_INFO_ 1

typedef std::chrono::milliseconds tms;

/* get the parameters from SHMWW, either PARA-1, or PARA-2 */
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
	} else {
		printf("Parameter set undefined, should be either 1 (80 bits secu) or 2 (128 bits secu), see SHMWW.\n");
		P.n = P.k = P.nprime = P.kprime = P.l = P.w1 = P.w2 = P.dGV = P.secu = 0;
	}
	return P;
}

/* Generates a random permutation matrix of dimension dim */
void genRandomPerm(mat_GF2 &P, int dim) {
	clear(P);
	int random_indices[dim] = {-1};
	int cpt=0;
	bool already_sampled;
	while (cpt<dim) {
		already_sampled=false;
		int prn = random()%dim;
		for (int i = 0; i < cpt; ++i) {
			if (prn == random_indices[i]) {
				already_sampled=true;
				break;
			}
		}
		if (!already_sampled) { 
			random_indices[cpt++]=prn;
		}
	}
	for (int i = 0; i < dim; ++i) {
		P[i][random_indices[i]]=GF2(1);
	}
}

/* Generate public key H, S, and secret key E */
void KeyGen(mat_GF2 &H, mat_GF2 &S, mat_GF2 &E, param_set P) {
	random(H, P.n-P.k, P.n);
	clear(E);
	for (int i = 0; i < P.l; ++i) {
		for (int j = 0; j < P.kprime; ++j) {
			E[j][i*P.nprime+j]=GF2(1);
		}
		for (int j = 0; j < P.kprime; ++j) {
			for (int k = 0; k < P.nprime-P.kprime; ++k) {
				E[j][i*P.nprime+P.kprime+1+k]=random_GF2();
			}
		}
	}
	mat_GF2 P1, P2;
	P1.SetDims(P.kprime, P.kprime);
	genRandomPerm(P1, P.kprime);
	P2.SetDims(P.n, P.n);
	genRandomPerm(P2, P.n);
	E=P1*E*P2;
	S = H*transpose(E);
}

/* Generate a random vector v of fixed Hamming weight v */ 
void genRandomWordOfFixedWeight(vec_GF2 &v, int length, int weight) {
	clear(v);
	int random_indices[weight] = {-1};
	int cpt=0;
	bool already_sampled;
	while (cpt<weight) {
		already_sampled=false;
		int prn = random()%length;
		for (int i = 0; i < cpt; ++i) {
			if (prn == random_indices[i]) {
				already_sampled=true;
				break;
			}
		}
		if (!already_sampled) { 
			random_indices[cpt++]=prn;
		}
	}
	for (int i = 0; i < weight; ++i) {
		v[random_indices[i]]=GF2(1);
	}
}

/* Produce an SHMWW signature (sigz, sigc), and stores the commitment phase in sigCom */
void Sign(vec_GF2 &sigz, vec_GF2 &sigc, vec_GF2 &sigCom, mat_GF2 E, mat_GF2 H, param_set P) {
	clear(sigz);
	clear(sigc);
	clear(sigCom);
	genRandomWordOfFixedWeight(sigc, P.kprime, P.w1);
	vec_GF2 e;
	e.SetLength(P.n);
	genRandomWordOfFixedWeight(e, P.n, P.w2);
	sigCom = H*e;
	sigz = sigc*E + e;
}

/* Verify the signature:
   returns true if signature verifies, false otherwise
*/
bool Verify(vec_GF2 sigz, vec_GF2 sigc, vec_GF2 sigCom, mat_GF2 H, mat_GF2 S, param_set P) {
	if (Hweight(sigz, P.n) > P.dGV) {
		cerr << "z too large\n";
		return false;
	}
	vec_GF2 tmp = H*sigz-S*sigc;
	bool identical = true;
	for (int i = 0; i < P.n-P.k; ++i) {
		if (tmp[i] != sigCom[i]) {
			cerr << "Inconsistent commited value " << i << "\n";
			identical=false;
		}
		if(!identical)
			return false;
	}
	return true;
}

/* Set the threshold for the attack */
uint64_t setThreshold(param_set P, int number_of_signatures) {
	return uint64_t(number_of_signatures * ((double(P.kprime)*double(P.l)/double(P.n))*(double(P.w1) / double(P.kprime) + (double(P.w2) / double(P.n)) * (1 - 2*double(P.w1) / double(P.kprime))) + ((double(P.nprime)-double(P.kprime))*double(P.l)/double(P.n))/2));
}

/* Sub-routine for the ISD:
   Initiate the constant part of matrix subH to be inverted
*/
void initSubH(mat_GF2 &subH, mat_GF2 H, uint16_t* random_indices, int nbColumns, param_set P) {
	for (int i = 0; i < P.n-P.k; ++i) {		
		for (int j = 0; j < nbColumns; ++j) {
			subH[i][j] = GF2(H[i][random_indices[j]]);
		}
	}
}

/* Sub-routine for the ISD:
   Complete the non-constant part of matrix subH to be inverted to get a square matrix
*/
void completeSubH(mat_GF2 &subH, mat_GF2 H, uint16_t* random_indices, int nbColumns, param_set P) {
	for (int i = 0; i < P.n-P.k; ++i) {		
		for (int j = nbColumns; j < P.n-P.k; ++j) {
			subH[i][j] = GF2(H[i][random_indices[j]]);
		}
	}
}

/* ISD part:
   try to invert the resulting matrix subH
   if subH not invertible, then abort
   else subH^{-1} * S is computed, and we check if this reveals unrecovered lines of the secret key
*/
void tryInvSubH(mat_GF2 &E, mat_GF2 subH, mat_GF2 S, bool* already_set, uint16_t *random_indices, int &current, int attempts, int &recovered, int maxWeight, param_set P) {
	GF2 det;
	mat_GF2 invSubH;
	inv(det, invSubH, subH);
	if (IsZero(det)) {
		return;
	}
	mat_GF2 tmpProd;
	tmpProd.SetDims(P.n-P.k, P.kprime);
	tmpProd = invSubH*S;
	for (int test = current; test < P.kprime; ++test) {
		if (!already_set[test]) {
			int column_weight = HweightC(tmpProd, P.n-P.k, test);
			if (column_weight!=0 && column_weight<=maxWeight) {
				for (int i = 0; i < P.n-P.k; ++i) {
					if (tmpProd[i][test]==1) {
						E[test][random_indices[i]] = 1;
					}
				}
				already_set[test] = true;
				++recovered;
			}
		}
	}
}

/* transform random_columns, a binary vector into random_indices the vector of non-zero indices */
int random_columnsTOrandom_indices(uint16_t *random_indices, vec_GF2 random_columns, param_set P) {
	int nbColumns=0;
	for (int i = 0; i < P.n; ++i)	{
		if (IsOne(random_columns[i])) {
			random_indices[nbColumns++] = i;
		}
	}
	return nbColumns;
}

/* Core of the cryptanalysis */ 
void recoverE(mat_GF2 &guessedSK, mat_GF2 H, mat_GF2 S, param_set P, vec_GF2 random_columns, uint64_t &nbtryinv, tms &tinv, uint64_t &nbrandomsamp, tms &trandomsampling){

	uint64_t maxWeight = P.l*(P.nprime-P.kprime);
	uint16_t random_indices[P.n-P.k] = {0};

	int nbColumns = random_columnsTOrandom_indices(random_indices, random_columns, P);

	mat_GF2 subH;
	subH.SetDims(P.n-P.k,P.n-P.k);
	initSubH(subH, H, random_indices, nbColumns, P);

	bool already_set[P.kprime] = {false};
	int current = 0, recovered = 0;
	while (recovered<P.kprime) {
		int startIndex = nbColumns;
		auto tstart = std::chrono::high_resolution_clock::now();
		while (startIndex < P.n-P.k) {
			uint16_t random_index = rand()%P.n; // dirty, should be using mt19937
			nbrandomsamp++;
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
		auto tend = std::chrono::high_resolution_clock::now(); 
		trandomsampling += std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);

		nbtryinv++;
		completeSubH(subH, H, random_indices, nbColumns, P);

		tstart = std::chrono::high_resolution_clock::now();
		tryInvSubH(guessedSK, subH, S, already_set, random_indices, current, nbtryinv, recovered, maxWeight, P);
		tend = std::chrono::high_resolution_clock::now(); 
		tinv += std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);

		for (int i = 0; i < P.kprime; ++i) {
			if (!already_set[i]) {
				current = i;
				break;
			}
		}
	}
}


int main(int argc, char const *argv[])
{

	if (argc != 4) {
		cerr << "USAGE: ./script X (for PARA-X) N (for N signatures) repeat (number of times)\n";
		exit(-1);
	}

	int instance = atoi(argv[1]);
	int nbSigs = atoi(argv[2]);
	int repetition = atoi(argv[3]);

	param_set P = getParameterSet(instance);

	// WARNING: Dirty patch to initialize to 0 timings!
	/* initialize timers */
	auto tstart = std::chrono::high_resolution_clock::now(); 
	auto tend = std::chrono::high_resolution_clock::now();
	// keygen
	auto tK = std::chrono::duration_cast<std::chrono::milliseconds>(tend - tend);
	// signing
	auto tS = std::chrono::duration_cast<std::chrono::milliseconds>(tend - tend);
	// verifying with success
	auto tVs = std::chrono::duration_cast<std::chrono::milliseconds>(tend - tend);
	// verifying with failure
	auto tVf = std::chrono::duration_cast<std::chrono::milliseconds>(tend - tend);
	//cryptanalysing
	tms tC[repetition] = {std::chrono::duration_cast<std::chrono::milliseconds>(tend - tend)};

	tms tinvsubh[repetition] = {std::chrono::duration_cast<std::chrono::milliseconds>(tend - tend)};

	tms trandomsampling[repetition] = {std::chrono::duration_cast<std::chrono::milliseconds>(tend - tend)};

	/* initializing variables */
	mat_GF2 H, realE, guessedES1, guessedE, guessedES3, guessedESI, S;
	vec_GF2 sigz, sigc, sigCom, random_columns;

	H.SetDims(P.n-P.k, P.n);

	realE.SetDims(P.kprime, P.n);
	guessedE.SetDims(P.kprime, P.n);

	S.SetDims(P.n-P.k, P.kprime);

	sigz.SetLength(P.n);
	
	sigc.SetLength(P.kprime);
	sigCom.SetLength(P.n-P.k);

	random_columns.SetLength(P.n);

	uint64_t threshold = setThreshold(P, nbSigs);

	uint64_t verif_success=0;
	uint64_t verif_failures=0;
	uint64_t recover_failures=0;

	uint64_t countRandomColumns[repetition]={0};
	uint64_t nbTryInvSubH[repetition]={0};
	uint64_t nbRandomSampling[repetition]={0};

	for (int iteration = 0; iteration < repetition; ++iteration) {
		tstart = std::chrono::high_resolution_clock::now();
		KeyGen(H, S, realE, P);
		tend = std::chrono::high_resolution_clock::now();
		tK += std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);

		//initialize weights
		uint64_t weights[P.n] = {0};
		int cpt = 0;
		clear(random_columns);
		clear(guessedE);

		for (int j = 0; j < nbSigs; ++j)
		{
			tstart = std::chrono::high_resolution_clock::now();
			Sign(sigz, sigc, sigCom, realE, H, P);
			tend = std::chrono::high_resolution_clock::now();
			tS += std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);

			tstart = std::chrono::high_resolution_clock::now();
			bool passed = Verify(sigz, sigc, sigCom, H, S, P);
			tend = std::chrono::high_resolution_clock::now();

			if (!passed) {
				cerr << "signature verification failed\n";
				verif_failures++;
				tVf += std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);
			} else {
				verif_success++;
				tVs += std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);
			}


			tstart = std::chrono::high_resolution_clock::now();
			//compute weights
			for(int k=0 ; k<P.n ; k++)
				if (sigz[k] == 1)
					weights[k]++;
			tend = std::chrono::high_resolution_clock::now();
			tC[iteration] += std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);
		}

		if (verif_failures!=0)
			cerr << verif_failures << "/" << nbSigs << " verification failures\n";



		tstart = std::chrono::high_resolution_clock::now();
		for (int j = 0; j < P.n; ++j) {
			if (weights[j] > threshold) {
				random_columns[j] = 1;
				cpt++;
			}
		}
		tend = std::chrono::high_resolution_clock::now();
		tC[iteration] += std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);

		countRandomColumns[iteration] += cpt;

		tstart = std::chrono::high_resolution_clock::now();
		recoverE(guessedE, H, S, P, random_columns, nbTryInvSubH[iteration], tinvsubh[iteration], nbRandomSampling[iteration], trandomsampling[iteration]);
		tend = std::chrono::high_resolution_clock::now();
		tC[iteration] += std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);


		bool secretKeyRecovered=true;
		for (int j = 0; j < P.kprime; ++j) {
			for (int k = 0; k < P.n; ++k) {
				if (guessedE[j][k] != realE[j][k]) {
					secretKeyRecovered=false;
					break;
				}
			}
		}

		if (!secretKeyRecovered)
			recover_failures++;
		if (!secretKeyRecovered)
			cout << "Number recovery failure(s) for : " << recover_failures << endl;
	}
	if (_SHMWW_INFO_) {
		cout << "===== SHMWW PARA-" << instance <<" =====\n";
		cout << "Generated " << repetition << " keys in " << tK.count() << " ms\n";
		cout << "Generated " << repetition*nbSigs << " signatures in " << tS.count() << " ms\n";
		cout << "Signatures passed verification " << verif_success << " times in " << tVs.count() << " ms\n";
		if (verif_failures > 0) {
			cout << "Signatures failed verification " << verif_failures << " times in " << tVf.count() << " ms\n";
		}
	}
	cout << "===== CRYPTANALYSIS =====\n";
	cout << "Cryptanalysis succesful: threshold=" << threshold << endl;
	if (_TOTAL_INFO_) {
		cout << "nbRandomColumns:  \t[";
		for (int i = 0; i < repetition-1; ++i) {
			cout << countRandomColumns[i] << "\t";
		}
		cout << countRandomColumns[repetition-1] << "]\n";
		cout << "nbRandomSampling: \t[";
		for (int i = 0; i < repetition-1; ++i) {
			cout << nbRandomSampling[i] << "\t";
		}
		cout << nbRandomSampling[repetition-1] << "]\n";
		cout << "nbTryInvSubH:     \t[";
		for (int i = 0; i < repetition-1; ++i) {
			cout << nbTryInvSubH[i] << "\t";
		}
		cout << nbTryInvSubH[repetition-1] << "]\n";
		cout << "time trandsamp:   \t[";
		for (int i = 0; i < repetition-1; ++i) {
			cout << trandomsampling[i].count() << "\t";
		}
		cout << trandomsampling[repetition-1].count() << "]\n";
		cout << "time trinv:       \t[";
		for (int i = 0; i < repetition-1; ++i) {
			cout << tinvsubh[i].count() << "\t";
		}
		cout << tinvsubh[repetition-1].count() << "]\n";
		cout << "-------------------------\n";

	}
	cout << "time break:           \t[";
	for (int i = 0; i < repetition-1; ++i) {
		cout << tC[i].count() << "\t";
	}
	cout << tC[repetition-1].count() << "]\n";

	return 0;
}
