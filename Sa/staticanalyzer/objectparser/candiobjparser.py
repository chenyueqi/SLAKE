#!/usr/bin/python
from __future__ import print_function
from os import mkdir, environ
from os.path import join
import subprocess

build_util = environ["FENGSHUI_PATH"] + "/src/Sa/utils/build_candi_obj.sh"
dedup_util = environ["FENGSHUI_PATH"] + "/src/Sa/utils/candi_dedup.sh"

class candiObjParser(object):
    def __init__(self, module_name="", bitcode_file=None, max_step=3, path_to_candi=None):
        if module_name=="":
            print("Please specify the module name")
            return
        if bitcode_file==None:
            print("Please specify the path to target llvm bitcode file")
            return
        if path_to_candi==None:
            print("Please specify the path to store candi results")
            return

        self._module_name = module_name
        self._bitcode_file = bitcode_file
        self._path_to_candi = path_to_candi
        self._max_step = max_step

    def run(self):
        for i in range(self._max_step):
            step_dump_path = join(self._path_to_candi, str(i+1)+"/")
            print("write candi obj to %s" % str(step_dump_path))
            cmd = [build_util, str(self._bitcode_file), str(step_dump_path), str(i+1)]
            subprocess.call(cmd)
            
            print("dedup candi results %s" % str(step_dump_path))
            cmd = [dedup_util, str(step_dump_path)]
            subprocess.call(cmd)
