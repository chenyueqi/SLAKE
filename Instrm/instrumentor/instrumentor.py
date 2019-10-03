#!/usr/bin/python
from __future__ import print_function
from os import chdir
from os.path import join
import subprocess
import sys

path_to_kernel="/home/yueqi/fengshui/vm/kernel/"
path_to_plugin=path_to_kernel+"scripts/gcc-plugins/fengshui_plugin.c"
path_to_plugin_h=path_to_kernel+"scripts/gcc-plugins/fengshui_plugin.h"
path_to_gcc="/home/yueqi/gcc/install/bin/gcc"

class insTrumentor(object):
    def __init__(self, flags, obj_name, path_to_candi):
        self._flags = flags
        self._obj_name = obj_name
        self._path_to_candi = path_to_candi
        self._subfield = []

    def run(self):
        self.modify_plugin()
        if self._flags == "DEREF":
            self.modify_panic_func()
        self.compile()

    def modify_plugin(self):
        if self._flags == "DEREF":
            self.decode_deref_chain()

        content = file(path_to_plugin, 'r').read().split('\n')
        for i in range(len(content)-1):
            if "const char* target_object_name"  in content[i]:
                content[i] = "const char* target_object_name = \"" + self._obj_name + "\";"
            
            if self._flags == "DEREF" and "const char* subfield_type_array[]" in content[i]:
                content[i] = "const char* subfield_type_array[] = {"
                for j in range(len(self._subfield)-1):
                    content[i] = content[i] + "\"" + str(self._subfield[j]) + "\","
                content[i] = content[i] + "\"" + str(self._subfield[-1]) + "\"};"

            if self._flags == "DEREF" and "static int subfield_type_array_len =" in content[i]:
                content[i] = "static int subfield_type_array_len = " + \
                                str(len(self._subfield)) + ";"

        new_plugin_file = open(path_to_plugin, 'w')
        for i in range(len(content)-1):
            print(content[i], file=new_plugin_file)

        plugin_file_h = open(path_to_plugin_h, 'w')
        print("#define INSTRUMENT", file=plugin_file_h)
        print("#define ALLOC_PLUGIN", file=plugin_file_h)
        if self._flags == "FREE":
            print("#define FREE_PLUGIN", file=plugin_file_h)
            print("#define RCU_PLUGIN", file=plugin_file_h)
        elif self._flags == "DEREF":
            print("#define DEREF_PLUGIN", file=plugin_file_h)
            print("#define RCU_PLUGIN", file=plugin_file_h)

    def decode_deref_chain(self):
        path_to_chain = join(self._path_to_candi, "struct."+self._obj_name)
        content = file(path_to_chain, 'r').read().split('\n')
        for i in range(len(content)-1):
            if "<" in content[i]:
                continue
            subfield = content[i].split(":")[0].split(".")[1]
            self._subfield.append(subfield)
    
    def modify_panic_func(self):
        pass

    def compile(self):
        chdir(path_to_kernel)

        cmd = ['make', 'clean']
        subprocess.call(cmd)

        cmd = ['make', 'CC=' + path_to_gcc, 'gcc-plugins']
        ret = subprocess.call(cmd)
        if ret != 0:
            print("Err when compiling gcc plugins")
            sys.exit(-1)

        cmd = ['make', 'CC=' + path_to_gcc, '-j16']
        ret = subprocess.call(cmd)
        if ret != 0:
            print("Err when compiling kernel")
            sys.exit(-1)

