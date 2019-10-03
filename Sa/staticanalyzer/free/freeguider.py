#!/usr/bin/python
from __future__ import print_function
from os import mkdir, environ, listdir, chdir
from os.path import exists, join, isfile
from shutil import rmtree, copy2
import subprocess

path_to_kernel="/home/yueqi/fengshui/vm/kernel/"
path_to_plugin=path_to_kernel+"scripts/gcc-plugins/fengshui_plugin.c"
path_to_plugin_h=path_to_kernel+"scripts/gcc-plugins/fengshui_plugin.h"

class freeGuider(object):
    def __init__(self, module_name, max_step, prefix, build_targets="", callgraph=None, path_to_TO="", path_to_free="", force_new = False, callgraph_mode = "semantic"):
        if callgraph==None:
            print("Please provide a callgraph for guider")
            return
        if path_to_TO=="":
            print("Please specify TO path")
            return
        if path_to_free=="":
            print("Please specify free path")
            return
        if force_new and build_targets=="":
            print("Please specify build target")
            return

        self._module_name = module_name
        self._max_step = max_step
        self._callgraph = callgraph
        self._build_targets = []
        self.init_build_targets(build_targets, prefix)

        self._path_to_TO = path_to_TO
        self._path_to_free = path_to_free

        self._force_new = force_new

        self._path_to_start = join(self._path_to_free, "start")
        if callgraph_mode == "semantic":
            self._path_to_end = join(self._path_to_free, "end_semantic")
        elif callgraph_mode == "type":
            self._path_to_end = join(self._path_to_free, "end_plain")
        elif callgraph_mode == "kint":
            self._path_to_end = join(self._path_to_free, "end_kint")
        elif callgraph_mode == "kint2":
            self._path_to_end = join(self._path_to_free, "end_kint2")
        self.setup_env()

    def init_build_targets(self, targets, prefix):
        target_list = targets.split(' ')
        for target in target_list:
            self._build_targets.append(join(prefix, target))

    def setup_env(self):
        print("create %s" % self._path_to_start)
        if not exists(self._path_to_start):
            mkdir(self._path_to_start)
            self._force_new = True
        elif self._force_new:
            rmtree(self._path_to_start)
            mkdir(self._path_to_start)

        print("create %s" % self._path_to_end)
        if exists(self._path_to_end):
            rmtree(self._path_to_end)
        mkdir(self._path_to_end)

    def run(self):
        self.set_start()
        self.get_end()

    def set_start(self):
        if not self._force_new:
            return
        path_to_target = join(self._path_to_TO, "target/"+str(self._max_step))
        files = [f for f in listdir(path_to_target) if isfile(join(path_to_target,f))]
        for f in files:
            self.invoke_sa_plugin(f)

    def get_end(self):
        files = [f for f in listdir(self._path_to_start) if isfile(join(self._path_to_start, f))]
        for f in files:
            start_func = []
            content = open(join(self._path_to_start, f)).read().split('\n')
            for caller in content:
                if caller == "":
                    continue
                start_func.append(caller)
            reachable = self._callgraph.backward_reachable_func(start_func)
            end_file = file(join(self._path_to_end, f), 'w')
            for func in reachable:
                print(func, file=end_file)

    def invoke_sa_plugin(self, obj_name):
        self.modify_sa_plugin(obj_name)
        self.build_kernel_target()

    def modify_sa_plugin(self, obj_name):
        content = file(path_to_plugin, 'r').read().split('\n')
        for i in range(len(content)-1):
            if "const char* target_object_name"  in content[i]:
                content[i] = "const char* target_object_name = \"" + obj_name + "\";"
            if "static char* dump_file_path =" in content[i]:
                content[i] = "static char* dump_file_path = (char*)\"" + \
                            join(self._path_to_start, obj_name) + "\";"
        new_plugin_file = open(path_to_plugin, 'w')
        for i in range(len(content)-1):
            print(content[i], file=new_plugin_file)

        plugin_file_h = open(path_to_plugin_h, 'w')
        print("#define STATICANA", file=plugin_file_h)
        print("#define FREE_PLUGIN", file=plugin_file_h)
        print("#define RCU_PLUGIN", file=plugin_file_h)

    def build_kernel_target(self):
        chdir(path_to_kernel)
        cmd = ["make", "clean"]
        subprocess.call(cmd)
        for target in self._build_targets:
            chdir(path_to_kernel)
            cmd = ["make", "CC=/home/yueqi/gcc/install/bin/gcc", str(target)]
            ret = subprocess.call(cmd)
            if ret != 0:
                print("build %s err", str(target))
                sys.exit(-1)
