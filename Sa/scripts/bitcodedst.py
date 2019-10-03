## This script generates kernel LLVM bitcode by using command in format of
## "make CC=clang ./kernel/umh.ll"
## Run this script in PATH_TO_KERNEL_SRC/
## Input: ./
## Output: bitcode files 
##         ./PATH_TO_KERNEL_SRC/ll_errout

from __future__ import print_function
from os import listdir
from os.path import isfile, isdir, join, splitext, exists
import sys
import subprocess

ll_errout = open("./ll_errout", 'w')

def build_ll(objfilepath):
    global ll_errout
    targetobjfile = splitext(objfilepath)[0]+".ll"
    if (exists(targetobjfile)):
        print("file exists")
        return
    cmd = ['make', 'CC=clang', targetobjfile]
    subprocess.call(cmd, stderr=ll_errout)

def process(kernel_path):
    dirs = [d for d in listdir(kernel_path) if isdir(join(kernel_path, d))]
    for i in range(len(dirs)):
        process(join(kernel_path, dirs[i]))
    files = [f for f in listdir(kernel_path) if isfile(join(kernel_path, f))]
    for i in range(len(files)):
        if (splitext(join(kernel_path, files[i]))[-1][1:] == "o"):
            print(join(kernel_path, files[i]))
            build_ll(join(kernel_path, files[i]))

if __name__ == '__main__':
    process(sys.argv[1])
    sys.exit(0)
