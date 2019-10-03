#!/usr/bin/python
from __future__ import print_function
from os import listdir, environ
from os.path import join, isfile

path_to_toc = environ["FENGSHUI_PATH"] + "/corpus/Sa/TOC/"

class callgraphParser(object):
    def __init__(self, module_name, callgraph, max_step):
        self._module_name = module_name
        self._callgraph = callgraph
        self._max_step = max_step

    def run(self):
        for i in range(self._max_step):
            self.do_parse(i)

    def do_parse(self, step):
        path_to_target = join(path_to_toc, self._module_name, "Target", str(step+1))
        print(path_to_target)
        path_to_final_target = join(path_to_toc, self._module_name, "FinalTarget", str(step+1))
        files = [f for f in listdir(path_to_target) if isfile(join(path_to_target, f))]
        for f in files:
            start_func = []
            callers = open(join(path_to_target, f)).read().split('\n')
            for caller in callers:
                if caller == "":
                    continue
                start_func.append(caller)
            reachable = self._callgraph.backward_reachable_func(start_func)
            print(reachable)
            """
            for func in reachable:
                if "SyS_" == func[0:3] or "sys_" == func[0:3]:
                    dump_file = open(join(path_to_final_target, f), 'a')
                    print(func, file=dump_file)
            """
