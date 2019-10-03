from __future__ import print_function
from os import listdir
from os.path import isfile, isdir, join, splitext, exists
import sys

template_path = "/home/yueqi/fengshui/dev/Fuzzing/Corpus/syzkaller/syscallTemplate/"
enable_syscalls = []
test_syscalls = ["ioctl"]
test_keywords = ["snd"]

def match(syscalls, keywords, sys_template):
    for s in syscalls:
        if s == "ioctl":
            for kw in keywords:
                if (kw.upper() in sys_template):
                    return True
        if s in sys_template:
            return True
    return False
            

def search_file(file_name, syscalls, keywords):
    global enable_syscalls
    syscall_template = open(file_name, 'r').read().split('\n')
    flag = False
    for s in syscall_template:
        if s == "":
            continue
        if match(syscalls, keywords, s):
            flag = True
            break
    if flag:
        for s in syscall_template:
            if s == "":
                continue
            enable_syscalls.append(s)
    

def extract(syscalls, keywords):
    global enable_syscalls
    files = [f for f in listdir(template_path) if isfile(join(template_path, f))]
    for f in files:
        search_file(join(template_path, f), syscalls, keywords)
    sys.stdout.write("\"enable_syscalls\":[")
    for i in range(len(enable_syscalls) - 2):
        sys.stdout.write("\"")
        sys.stdout.write(enable_syscalls[i])
        sys.stdout.write("\",")
    if len(enable_syscalls) == 0:
        sys.stdout.write("]")
        return
    sys.stdout.write("\"")
    sys.stdout.write(enable_syscalls[len(enable_syscalls)-1])
    sys.stdout.write("\"]")
    

if __name__ == '__main__':
    extract(test_syscalls, test_keywords)
    sys.exit(0)
