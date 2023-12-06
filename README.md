# CBMCsynth

Basic synthesizer using CBMC datatypes

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
Compile CBMCsynth
~~~
cd CBMCsynth/src
make
~~~

The binary is found at `CBMCsynth/src/CBMCsynth/CBMCsynth`. 

To run CBMCsynth on a SyGuS file and do basic cegis:
~~~
CBMCsynth file.sl --cegis
~~~


