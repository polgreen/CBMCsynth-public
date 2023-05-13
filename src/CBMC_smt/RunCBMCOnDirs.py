import os
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import subprocess
from tqdm import tqdm
import concurrent.futures

def run_example(cmd, input_file, output_file, idx, all):
    try :
        subprocess.run([cmd, input_file , output_file], capture_output=True, text=True, timeout=20)
        print("Ran {}/{}\t {}".format(idx, all, input_file))
    except subprocess.TimeoutExpired as e:
        return
    return


def main():
    n_cores = int(sys.argv[1])
    cmd = sys.argv[2]
    base_input = sys.argv[3]
    base_output = sys.argv[4]

    result = [str(x) for x in list(Path(base_input).rglob("*.smt2"))]

    args_list = []
    for file in result:
        output_file = str(os.path.join(base_output, os.path.relpath(file, base_input)))[:-4]+"sl"
        args_list.append((cmd, file, output_file))

    with ThreadPoolExecutor(max_workers=n_cores) as executor:
        for idx, x in enumerate(args_list):
            res = executor.submit(run_example, *x, idx, len(args_list))


if __name__ == '__main__':
    main()