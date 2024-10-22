# 🌟 CBMCsynth

CBMCsynth is a synthesizer using the CBMC datatypes. It implements a number of prototype research algorithms including a **top-down search** and **A\* search**, both of which can be guided by a **probabilistic grammar**, and can make calls to an **LLM** during the synthesis process in order to update this grammar.

⚠️ Note that this is a prototype!

## 🛠️ Building
### Prerequisites: 
1. **CBMC Prerequisites**: All dependencies required by CBMC must be installed. Follow the instructions here: [CBMC Compilation Guide](https://github.com/diffblue/cbmc/blob/develop/COMPILING.md). 
2. **Z3 Solver**: Add Z3 to your system’s `$PATH`. Download Z3 here:[Z3 Releases](https://github.com/Z3Prover/z3/releases).


### Steps
1. **Update the Git Submodule (CBMC)**:
   ```bash
   git submodule init
   git submodule update
   ```

2. **Install CBMC Dependencies**:
Ensure you have **Flex**, **Bison**, and **GNU Make** installed. Check CBMC’s [Compiling Guide](https://github.com/diffblue/cbmc/blob/develop/COMPILING.md) for details.

3. **Download and Patch Minisat, Compile CBMC**:
   ```bash
    cd lib/cbmc/src
    make minisat2-download
    make
   ```

4. **Compile CBMCsynth**:
   ```bash
    cd ../../../src/CBMCsynth
    make
   ```

### Result Binary

Once compiled, the binary will be located at:  
`CBMCsynth-public/src/CBMCsynth/CBMCsynth`. 

## 🚀 Running CBMCsynth

To run CBMCsynth on a SyGuS file and do A* search, guided by a set of weights on each production rule:

```bash
    CBMCsynth max2.sl --astar --probs maxprobs.txt
```
- **`maxprobs.txt`** should contain one integer per production rule, each on a new line. You can find sample files (`max2.sl` and `maxprobs.txt`) at the root of this repository.

### Top-Down Search with Probabilities File

To run CBMCsynth on a SyGuS file and do top-down search, guided by a probabilities file:

```bash
    CBMCsynth max2.sl --top-down-cegis --probs maxprobs.txt
```

### Incorporating LLM Calls

To add calls to the LLM during the synthesis phase, the command line option `--use-LLM' 
can be added. 
```bash
CBMCsynth max2.sl --top-down-cegis --probs maxprobs.txt --use-LLM
```
You will need to have an openai key stored in your **OpenAI API Key** environment variable.
```bash
export OPENAI_API_KEY=your_openai_key_here
```


## 📁 Input File Structure

- **`max2.sl`**: Example SyGuS file for synthesis.
```lisp
(set-logic LIA)

(synth-fun max2 ((x Int) (y Int)) Int
    ((Start Int) (StartBool Bool))
    ((Start Int (x y (ite StartBool Start Start)))
    (StartBool Bool ((>= Start Start)))))

(declare-var x Int)
(declare-var y Int)
(constraint (>= (max2 x y) x))
(constraint (>= (max2 x y) y))
(constraint (or (= x (max2 x y)) (= y (max2 x y))))

(check-synth)
```

- **`maxprobs.txt`**: Weights for each production rule in the grammar.
```lisp
1
3
1
1
```

## 📌 Citation

If you use **CBMCsynth** in your research, please cite the following paper:
```bibtex
@InProceedings{LLM-SYGUS,
author="Li, Yixuan
and Parsert, Julian
and Polgreen, Elizabeth",
title="Guiding Enumerative Program Synthesis with Large Language Models",
booktitle="Computer Aided Verification",
year="2024",
publisher="Springer Nature Switzerland",
address="Cham",
pages="280--301",
isbn="978-3-031-65630-9",
url={https://doi.org/10.1007/978-3-031-65630-9_15},
doi={10.1007/978-3-031-65630-9_15},
location = {Montreal, Canada},
series = {CAV 2024}
}
```
