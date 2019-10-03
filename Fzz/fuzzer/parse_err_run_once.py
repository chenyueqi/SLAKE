from __future__ import print_function
import sys
from os import listdir
from os.path import isfile, join


def process():
    path_to_err_run_once = "./err_run_once/"
    path_to_result = "./allocation_site/"
    files = [f for f in listdir(path_to_err_run_once) if isfile(join(path_to_err_run_once, f))]
    for i in range(len(files)):
        logs = file(join(path_to_err_run_once, files[i]), 'r').read().split('\n')
        dump_file = open(join(path_to_result, files[i]), 'w')
        for log in logs:
            if "find an allocation site in function" in log:
                dump_file.write(log)
                dump_file.write("\n")

if __name__ == '__main__':
    process()
    sys.exit(0)
