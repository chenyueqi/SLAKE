#!/usr/bin/python
from __future__ import print_function
from os import mkdir, environ, listdir
from os.path import exists, join, isfile
from shutil import rmtree, copy2

class allocGuider(object):
    def __init__(self, module_name, max_step, callgraph=None, path_to_TO="", path_to_alloc="", callgraph_mode="semantic"):
        if callgraph==None:
            print("Please provide a callgraph for guider")
            return
        if path_to_TO=="":
            print("Please specify TO path")
            return
        if path_to_alloc=="":
            print("Please specify alloc path")
            return

        self._module_name = module_name
        self._max_step = max_step
        self._callgraph = callgraph
        self._path_to_TO = path_to_TO
        self._path_to_alloc = path_to_alloc

        self._path_to_start = join(self._path_to_alloc, "start")
        if (callgraph_mode == "semantic"):
            self._path_to_end = join(self._path_to_alloc, "end_semantic")
        elif (callgraph_mode == "type"):
            self._path_to_end = join(self._path_to_alloc, "end_plain")
        elif (callgraph_mode == "kint"):
            self._path_to_end = join(self._path_to_alloc, "end_kint")
        elif (callgraph_mode == "kint2"):
            self._path_to_end = join(self._path_to_alloc, "end_kint2")
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
        # for alloc, simply re-use parser's result
        path_to_target = join(self._path_to_TO, "target/"+str(self._max_step))
        files = [f for f in listdir(path_to_target) if isfile(join(path_to_target,f))]
        for f in files:
            copy2(join(path_to_target, f), self._path_to_start)

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
