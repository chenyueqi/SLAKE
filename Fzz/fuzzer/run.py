#!/usr/bin/python
from __future__ import print_function
from os import environ, chdir
from os.path import exists
import sys
import subprocess
import time

def instrument(t):
    workdir = environ["FENGSHUI_PATH"] + "/src/Instrm/instrumentor/"
    chdir(workdir)
    print("[+] Instrumenting %s" % t)
    cmd = ['./test.py', t]
    outfile = open(workdir + "/stdout", 'w')
    errfile = open(workdir + "/stderr", 'w')
    ret = subprocess.call(cmd, stdout = outfile, stderr = errfile)
    if ret != 0:
        print("[-] Instrument error in %s" % t)
        return -1
    return 0

def fuzzing(t):
    workdir = environ["FENGSHUI_PATH"] + "/testcases/" + t + "/alloc/"
    if not exists(workdir):
        print("[-] Please create workdir for %s" % t)
        return
    chdir(workdir)
    timeout = 120*60
    delay = 10
    print("[+] Fuzzing %s" % t)
    cmd = ['./run.sh']
    outfile = open(workdir + "/stdout", 'w')
    errfile = open(workdir + "/stderr", 'w')
    task = subprocess.Popen(cmd, stdout = outfile, stderr=errfile, shell=True)
    while task.poll() is None and timeout > 0:
        flag = False
        logs = file(workdir + "/stderr", 'r').read().split('\n')
        for log in logs:
            if "kernel panic: find allocation of target object exec-ready" in log:
                flag = True
                break
        if flag:
            break
        time.sleep(delay)
        timeout -= delay
    print("[+] Finish fuzzing ")

    if timeout <= 0:
        print("[-] Fail to allocate %s" % t)
    else:
        print("[+] Succeed in allocating %s" % t)

    task.kill()
    cmd = ['pidof', 'syz-manager']
    pid = subprocess.check_output(cmd)
    pid = pid.split('\n')[0]
    cmd = ['kill', '-9', pid]
    subprocess.call(cmd)

if __name__ == '__main__':
    target_list = file("./queue", 'r').read().split('\n')
    for t in target_list:
        if t == "":
            continue
        print("[+] Running", t)
        if instrument(t) == -1:
            continue
        fuzzing(t)
