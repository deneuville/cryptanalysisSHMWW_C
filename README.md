# cryptanalysisSHMWW_C
C implementation of the cryptanalysis of Song *et al.* code-based signature scheme, publishesd in [Theoretical Computer Science](https://doi.org/10.1016/j.tcs.2020.05.011)

This repository hosts:
1. a (raw) sage implementation of the code-based signature scheme proposed by Song, Huang, Mu, Wu, and Wang [Theoretical Computer Science](https://doi.org/10.1016/j.tcs.2020.05.011) (file `SHMWW.sage`),
1. a C++ cryptanalysis implementation that recovers the secret key from a few hundreds of signatures (file `cryptanalysis.cpp`). 

## Generating an instance and signatures

Song *et al.* proposed two sets of parameters: PARA-1 and PARA-2 respectively targeting 80 and 128 bits of classical security (see Table 1 of [SHMWW]).

To generate an instance of PARA-`X` along with `N` signatures for that instance:
1. Modify line 4 of `SHMWW.sage` into `__PARAMETER_SET__ = X`
1. Modify line 5 of `SHMWW.sage` into `__NUMBER_OF_SIGS__ = N`
1. Run `sage`
1. Load and run the script: `%runfile SHMWW.sage`

This script generates three files:
1. `samples/PARA-Xpk` that contains the public key under the form
```
first line of H\n
      ...      \n
last  line of H\n
first line of S\n
      ...      \n
last  line of S\n
```
2. `samples/PARA-Xsk` that contains the secret key under the form
```
first line of E\n
      ...      \n
last  line of E\n
```
3. `PARA-Xsig` that contains N signatures under the form
```
vector z of length n\n
```

For cryptanalytic purposes, the use of a weight restricted hash (WRH) function is not mandatory. Instead, we generate the challenge as a vector c of same lenght (h') and weight (w_1) as the output of the WRH function described in [SHMWW].

Verbose mode can be turned off by setting `__VERBOSE__ = 0` on line 8 of `SHMWW.sage` and commenting out `#define VERBOSE` on line 4 of `cryptanalysis.cpp`. Manual configuration on the fly can be turned on by setting `__MANUAL_SETUP__ = 1` on line 7 of `SHMWW.sage`.

## Running the cryptanalysis

To run the cryptanalysis, run:
1. `make`
1. `./breakSHMWW PARAM_SET NUMBER_OF_SIGNATURES`

The scripts reads `pk` and the signatures, prints the number of lines of `sk` recovered and when done, compares the recovered secret key to the original one.

Depending on the instance, the number of signatures available and your machine, the scripts may take more than 30 minutes to complete.

The cryptanalysis requires [`NTL`](https://www.shoup.net/ntl/). Our version of `NTL` was compiled using [`GMP`](https://gmplib.org/) but the latter library is not mandatory for the cryptanalysis, simply remove it from the `Makefile` if you have a `GMP`-free version of `NTL` 

## Technical details and reference

The technical details about the cryptanalysis will be soon provided in a complete paper.

[SHMWW] Song, Y., Huang, X., Mu, Y., Wu, W., & Wang, H. (2020). *A code-based signature scheme from the Lyubashevsky framework*. Theoretical Computer Science.
