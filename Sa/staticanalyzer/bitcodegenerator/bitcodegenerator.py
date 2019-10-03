#!/usr/bin/python
from __future__ import print_function
from os.path import exists, join
from os import remove, chdir, getcwd, environ
import subprocess

path_to_bitcode_kernel = environ["FENGSHUI_PATH"] + "/corpus/Sa/clang_kernel/"

class bitcodeGenerator(object):
    def __init__(self, targets, prefix, output_file):
        self._cc = "clang"
        self._cflags = "-O0 -fno-inline-functions"
        self._target = []
        self._prefix = prefix
        self.init_targets(targets, prefix)
        self._output_file = output_file

    def init_targets(self, targets, prefix):
        target_list = targets.split(' ')
        for target in target_list:
            self._target.append(join(prefix, target.split('.o')[0]+'.ll'))
    
    def build(self):
        if self._target == []:
            return
        else:
            for target in self._target:
                self.build_target(target)
        self.link()

    def build_target(self, target):
        if exists(join(path_to_bitcode_kernel, target)):
            remove(join(path_to_bitcode_kernel, target))
        if self._cflags == "":
            CC = "CC=\""+ str(self._cc) + "\""
        else:
            CC = "CC=\""+ str(self._cc) + " " + str(self._cflags) + "\""
        chdir(path_to_bitcode_kernel)
        cmd = ['make ' +  CC + ' ' +  str(target)]
        subprocess.call(cmd, shell=True)

    def link(self):
        #if len(self._target) == 1:
        #    return
        cmd = ['llvm-link', '-o', self._output_file]
        for target in self._target:
            cmd.append(target[len(self._prefix):])
        chdir(join(path_to_bitcode_kernel, self._prefix))
        getcwd()
        subprocess.call(cmd)
