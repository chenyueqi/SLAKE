#!/usr/bin/python
from __future__ import print_function
from os import mkdir, environ, listdir
from os.path import exists, join, isfile
from shutil import rmtree, copy2

class type1Guider(object):
    def __init__(self, module_name, callgraph=None, path_to_TO="", path_to_type1="", callgraph_mode = "semantic"):
        if callgraph==None:
            print("Please provide a callgraph for guider")
            return
        if path_to_TO=="":
            print("Please specify TO path")
            return
        if path_to_type1=="":
            print("Please specify alloc path")
            return

        self._module_name = module_name
        self._callgraph = callgraph
        self._path_to_TO = path_to_TO
        self._path_to_type1 = path_to_type1

        self._path_to_start = join(self._path_to_type1, "start")
        if callgraph_mode == "semantic":
            self._path_to_end = join(self._path_to_type1, "end_semantic")
        elif callgraph_mode == "type":
            self._path_to_end = join(self._path_to_type1, "end_plain")
        elif callgraph_mode == "kint":
            self._path_to_end = join(self._path_to_type1, "end_kint")
        elif callgraph_mode == "kint2":
            self._path_to_end = join(self._path_to_type1, "end_kint2")
            
        self.setup_env()

    def setup_env(self):
        print("create %s" % self._path_to_start)
        if exists(self._path_to_start):
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
        # for type1, simply re-use parser's result
        path_to_type1_gcc = join(self._path_to_TO, "type1/gcc")
        copy2(path_to_type1_gcc, self._path_to_start)

    def get_end(self):
        path_to_type1_gcc = join(self._path_to_start, "gcc")
        content = open(path_to_type1_gcc).read().split('\n')
        counter = 0
        for func in content:
            if func == "":
                continue
            start_func = []
            start_func.append(func)
            reachable = self._callgraph.backward_reachable_func(start_func)
            end_file = file(join(self._path_to_end, str(counter)), 'w')
            for reachable_func in reachable:
                print(reachable_func, file=end_file)
            counter = counter + 1
