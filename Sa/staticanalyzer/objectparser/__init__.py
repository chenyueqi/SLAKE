#!/usr/bin/python
from __future__ import print_function
from os import mkdir
from os import listdir
from os import remove
from os import environ
from os.path import exists
from os.path import join
from os.path import isfile
import shutil
import candiobjparser
import slabobjparser
import type1objparser

class objParser(object):
    """ find objects to be used in exploiation in kernel module
    Currently, we are interested in following objects
    1. can dereference function pointer in multiple steps, for example, file->f_op_llseek
    2. can be used for heap spray in use-after-free vulnerability, for example, user_key_payload
    """

    def __init__(self, module_name, prefix, bitcode_file, \
                build_targets, max_step, path_to_TO):
        self._module_name = module_name
        self._bitcode_file = bitcode_file
        self._max_step = max_step # dereference step
        self._slab_obj = []
        self._candi_obj = []
        self._build_targets = []
        self._path_to_TO = path_to_TO
        self.init_build_targets(build_targets, prefix)

        self._path_to_candi = join(self._path_to_TO, "candi")
        self._path_to_slab = join(self._path_to_TO, "slab")
        self._path_to_target = join(self._path_to_TO, "target")
        self._path_to_type1 = join(self._path_to_TO, "type1")

        self.setup_env()

    def init_build_targets(self, targets, prefix):
        """ turn build targets in format of string to list
        """
        target_list = targets.split(' ')
        for target in target_list:
            self._build_targets.append(join(prefix, target))

    def setup_env(self):
        """ Build environment to storing results of both (1) and (2)
        """
        print("create %s" % self._path_to_candi)
        if exists(self._path_to_candi):
            shutil.rmtree(self._path_to_candi)
        mkdir(self._path_to_candi)
        for i in range(self._max_step):
            mkdir(join(self._path_to_candi, str(i+1)))

        print("create %s" % self._path_to_slab)
        if exists(self._path_to_slab):
            shutil.rmtree(self._path_to_slab)
        mkdir(self._path_to_slab)
        mkdir(join(self._path_to_slab, "llvm"))
        mkdir(join(self._path_to_slab, "mix"))

        print("create %s" % self._path_to_target)
        if exists(self._path_to_target):
            shutil.rmtree(self._path_to_target)
        mkdir(self._path_to_target)
        for i in range(self._max_step):
            mkdir(join(self._path_to_target, str(i+1)))

        print("create %s" % self._path_to_type1)
        if exists(self._path_to_type1):
            shutil.rmtree(self._path_to_type1)
        mkdir(self._path_to_type1)

    def run(self):
        """
        Run candi parser and slab parser and then interset their results (1)
        Run type1 parser (2)
        """
        candi_parser = candiobjparser.candiObjParser(
                                self._module_name, 
                                self._bitcode_file, 
                                self._max_step,
                                self._path_to_candi)
        candi_parser.run()

        slab_parser = slabobjparser.slabObjParser(
                                self._module_name, 
                                self._bitcode_file, 
                                self._build_targets,
                                self._path_to_slab)
        slab_parser.run()
        self.intersect()

        type1_parser = type1objparser.type1ObjParser(
                                self._build_targets,
                                self._path_to_type1)
        type1_parser.run()

    def intersect(self):
        self.load_slab()
        for i in range(self._max_step):
            self.load_candi(i)
            self.dump_result(i)
            
    def load_slab(self):
        path_to_slab_result = join(self._path_to_slab, "mix")
        files = [f for f in listdir(path_to_slab_result) if isfile(join(path_to_slab_result, f))]
        for f in files:
            obj_name = f
            self._slab_obj.append(obj_name)

    def load_candi(self, curr_step):
        self._candi_obj = []
        path_to_candi_result = join(self._path_to_candi, str(curr_step+1))
        files = [f for f in listdir(path_to_candi_result) if isfile(join(path_to_candi_result, f))]
        for f in files:
            if "union." in f:
                continue
            obj_name = f.split('struct.')[1]
            self._candi_obj.append(obj_name)

    def dump_result(self, curr_step):
        path_to_dump = join(self._path_to_target, str(curr_step+1))
        path_to_slab_result = join(self._path_to_slab, "mix")
        for obj in self._candi_obj:
            if obj in self._slab_obj:
                shutil.copy2(join(path_to_slab_result, obj), path_to_dump)
