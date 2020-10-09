# cryptanalysisSHMWW_C
C implementation of the cryptanalysis of Song *et al.* code-based signature scheme, publishesd in [Theoretical Computer Science](https://doi.org/10.1016/j.tcs.2020.05.011)

This repository hosts:
1. a C++ implementation of the code-based signature scheme proposed by Song, Huang, Mu, Wu, and Wang [Theoretical Computer Science](https://doi.org/10.1016/j.tcs.2020.05.011),
1. a C++ cryptanalysis implementation that recovers the secret key from 10 signatures or more.

## Compiling the software

To run the software, you should have `gf2x` and `ntl` installed at a standard location `/usr/local` (modify the `Makefile` accordingly otherwise).

Simply run `make`.

## Generating an instance along with signatures

Song *et al.* proposed two sets of parameters: PARA-1 and PARA-2 respectively targeting 80 and 128 bits of classical security (see Table 1 of [SHMWW]).

To generate an instance of PARA-`X` along with `N` signatures for that instance, run:
```./breakSHMWW X N 1```

If you want to run `K` such instances (iteratively), replace `1` in the previous command by `K`:
```./breakSHMWW X N K```

To reduce the burden of manipulating files, the keys and signatures are voluntary not stored.

For cryptanalytic purposes, the use of a weight restricted hash (WRH) function is not mandatory. Instead, we generate the challenge as a vector c of same lenght (h') and weight (w_1) as the output of the WRH function described in [SHMWW].

By default, the software provides information about:
1. The time it took to generate the keypair (SHMWW-related)
1. The time it took to generate the signatures (SHMWW-related)
1. The time it took to verify those signatures (SHMWW-related)
1. Timings and sub-routine internal information (internal-related)
1. The time it took to cryptanalyse the instance.

If you are not interrested in internal-related information, modify line `5` of `cryptanalysisSHMWW.cpp` the following way:
```#define _TOTAL_INFO_ 0```

If you are not interrested in SHMWW-related information, modify line `6` of `cryptanalysisSHMWW.cpp` the following way:
```#define _SHMWW_INFO_ 0```

Depending on the instance, the number of signatures available and your machine, the scripts may take more than 30 minutes to complete (almost for PARA-2).

## Example of output for each parameter set

Some sample cryptanalysis results using 100 signatures for both parameter sets. (obtained with Intel(R) Core(TM) i9-9980HK CPU @ 2.40GHz)

```
deneuville@deneuville$ ./breakSHMWW 1 100 10
===== SHMWW PARA-1 =====
Generated 10 keys in 1572 ms
Generated 1000 signatures in 903 ms
Signatures passed verification 1000 times in 1000 ms
===== CRYPTANALYSIS =====
Cryptanalysis succesful: threshold=20
nbRandomColumns:  	[860	866	842	838	848	851	839	847	841	860]
nbRandomSampling: 	[256802	410057	625942	323842	404621	464803	353672	411196	264368	256344]
nbTryInvSubH:     	[35	56	85	44	55	63	48	56	36	35]
time trandsamp:   	[103	171	255	132	165	189	144	168	108	105]
time trinv:       	[14378	21139	31061	17791	19403	21773	17999	19550	17573	14560]
-------------------------
time break:           	[15029	22240	32734	18655	20486	23014	18945	20658	18282	15254]
```

```
deneuville@deneuville$ ./breakSHMWW 2 100 10
===== SHMWW PARA-2 =====
Generated 10 keys in 5868 ms
Generated 1000 signatures in 3088 ms
Signatures passed verification 1000 times in 3197 ms
===== CRYPTANALYSIS =====
Cryptanalysis succesful: threshold=19
nbRandomColumns:  	[1797	1784	1775	1768	1783	1789	1754	1798	1788	1780]
nbRandomSampling: 	[1174723	1087998	1133421	1253342	1486110	1380933	898625	1438866	835456	1381506]
nbTryInvSubH:     	[80	74	77	85	101	94	61	98	57	94]
time trandsamp:   	[980	888	921	1027	1239	1185	733	1164	677	1122]
time trinv:       	[263612	246354	277336	267325	313796	305307	214645	309996	231808	312888]
-------------------------
time break:           	[269607	251746	282868	273585	321355	312491	219079	316961	235884	319678]
```

## Technical details and reference

The technical details about the cryptanalysis will be soon provided in a complete paper. Some preliminary information can be found in [ADG20, BKPS20]. A merged paper has been submitted to [Designs, Codes and Cryptography](https://www.springer.com/journal/10623).

[ADG20] Aragon, N., Deneuville, J.-C., & Gaborit, P. (2020). *Another code-based adaptation of Lyubashevsky’s signature cryptanalysed*. [ePrint2020/923](https://eprint.iacr.org/2020/923)

[BKPS20] Baldi, M., Khathuria, K., Persichetti, E., Santini, P. (2020). *Cryptanalysis of a Code-Based Signature Scheme Based on the Lyubashevsky Framework*. [ePrint2020/905](https://eprint.iacr.org/2020/905)

[SHMWW] Song, Y., Huang, X., Mu, Y., Wu, W., & Wang, H. (2020). *A code-based signature scheme from the Lyubashevsky framework*. Theoretical Computer Science.


