# CBMC_smt

### Building
Prerequisites: All prerequisties for CBMC must be installed (https://github.com/diffblue/cbmc/blob/develop/COMPILING.md). Plus Z3 must be added to $PATH.  
- Z3 downloads:https://github.com/Z3Prover/z3/releases

update the gitsubmodule (CBMC)
~~~
git submodule init
git submodule update
~~~
Ensure you have the dependencies for CBMC installed (Flex and Bison, and GNU make) (see https://github.com/diffblue/cbmc/blob/develop/COMPILING.md). Then 
download and patch minisat, and compile cbmc as follows:
~~~
cd lib/cbmc/src
make minisat2-download
make
~~~
Compile CBMC_smt
~~~
cd CBMC_smt/src
make
~~~

The binary is found at `CBMC_smt/src/CBMC_smt/CBMC_smt`. 

To run CBMC_smt on a file:
~~~
CBMC_smt file.smt2
~~~


use/install     
https://github.com/crashoz/uuid_v4     
version:  v1.0.0


### Modifying

All the useful code is in src/CBMC_smt/smt2_frontend.cpp. Start here.
