# CBMCsynth

CBMCsynth is a synthesizer using the CBMC datatypes. It implements a number of prototype research algorithms including a top-down search and A* search, both of which can be guided by a probabilistic grammar, and can make calls to an LLM during the synthesis process in order to update this grammar.

Note that this is a prototype!

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

### Running

To run CBMCsynth on a SyGuS file and do A* search, guided by a set of weights on each production rule:
~~~
CBMCsynth max2.sl --astar --probs maxprobs.txt
~~~
The file maxprobs.txt should contain a single integer for each proudction rule in the grammar. Each integer should be on a new line. You can find max2.sl and maxprobs.txt at the top level of this git repository.

To run CBMCsynth on a SyGuS file and do top-down search, guided by a probabilities file:
~~~
CBMCsynth max2.sl --top-down-cegis --probs maxprobs.txt
~~~

To add calls to the LLM during the synthesis phase, the command line option `--use-LLM' can be added. You will need to have an openai key stored in your `OPENAI_API_KEY` environment variable.
