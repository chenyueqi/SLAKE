#!/usr/bin/python
from __future__ import print_function
from os.path import join, isfile
from os import chdir, listdir, environ
import sys
import subprocess

path_to_kernel=environ["FENGSHUI_PATH"] + "/vm/kernel/"
path_to_plugin=path_to_kernel+"scripts/gcc-plugins/fengshui_plugin.c"
path_to_plugin_h=path_to_kernel+"scripts/gcc-plugins/fengshui_plugin.h"
allocsite_util=environ["FENGSHUI_PATH"] + "/src/Sa/utils/build_alloc_site.sh"

class slabObjParser(object):
    def __init__(self, module_name, bitcode_file, build_targets, path_to_slab):
        self.obj_dict = {}
        self.gcc_result_path = path_to_slab + "/gcc"
        self.llvm_result_path = path_to_slab + "/llvm/"
        self.sum_result_path = path_to_slab + "/mix/"

        self.launch_gcc_plugin(build_targets)
        self.launch_llvm_pass(bitcode_file)

    """
    methods for gcc plugin
    """
    def launch_gcc_plugin(self, build_targets):
        self.modify_gcc_plugin()
        self.build_kernel_target(build_targets)

    def modify_gcc_plugin(self):
        plugin_file = file(path_to_plugin, 'r').read().split('\n')
        for i in range(len(plugin_file)-1):
            if "static char* dump_file_path =" in plugin_file[i]:
                plugin_file[i] = "static char* dump_file_path = (char*)\"" + \
                                self.gcc_result_path +  "\";"
                print("write gcc result to %s" % self.gcc_result_path)
                break
        new_plugin_file = open(path_to_plugin, 'w')
        for i in range(len(plugin_file)-1):
            print(plugin_file[i], file=new_plugin_file)

        plugin_file_h = open(path_to_plugin_h, 'w')
        print("#define STATICANA", file = plugin_file_h)
        print("#define ALLOC_PLUGIN", file = plugin_file_h)

    def build_kernel_target(self, build_targets):
        self.make_clean()
        for target in build_targets:
            print("building target %s" % str(target))
            chdir(path_to_kernel)
            cmd = ["make", "CC=/home/yueqi/gcc/install/bin/gcc", str(target)]
            ret = subprocess.call(cmd)
            if ret != 0:
                print("build target err")
                # sys.exit(-1)

    def make_clean(self):
        print("clean kernel")
        chdir(path_to_kernel)
        cmd = ["make", "clean"]
        subprocess.call(cmd)

    """
    methods for llvm pass
    """
    def launch_llvm_pass(self, bitcode_file):
        print("write llvm result to %s" % self.llvm_result_path)
        cmd = [allocsite_util, bitcode_file, self.llvm_result_path]
        subprocess.call(cmd)
    
    """
    methods for summarizing results
    """
    def run(self):
        self.collect_gcc()
        self.collect_llvm()
        self.dump_result()

    def collect_gcc(self):
        logs = open(self.gcc_result_path).read().split('\n')
        for log in logs:
            if log == "":
                continue
            [obj_name, alloc_func, func_name] = log.split(':')
            if self.obj_dict.has_key(obj_name) == False:
                self.obj_dict[obj_name] = []
            if func_name in self.obj_dict[obj_name]:
                continue
            self.obj_dict[obj_name].append(func_name)
    
    def collect_llvm(self):
        files = [f for f in listdir(self.llvm_result_path) if isfile(join(self.llvm_result_path, f))]
        for f in files:
            if "union." in f:
                continue
            obj_name = f.split('struct.')[1]
            if "." in obj_name:
                obj_name = obj_name.split('.')[0]
            if self.obj_dict.has_key(obj_name) == False:
                continue
            logs = open(join(self.llvm_result_path, f), 'r').read().split('\n')
            for func in logs:
                if func == "" or func in self.obj_dict[obj_name]:
                    continue
            	self.obj_dict[obj_name].append(func)

    def dump_result(self):
        for obj_name in self.obj_dict:
            dump_file = open(self.sum_result_path+obj_name, 'a')
            for funcs in self.obj_dict[obj_name]:
                print(funcs, file=dump_file)	
