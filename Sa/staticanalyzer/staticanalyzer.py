#!/usr/bin/python
from __future__ import print_function
from os import mkdir, environ
from os.path import exists, join
import sys
sys.path.append("callgraph/")
sys.path.append("objectparser/")
sys.path.append("bitcodegenerator/")
import bitcodegenerator as bcgenerator
import objectparser as obparser
import callgraph as cg
import alloc
import free
import deref
import type1
import fzzhinter

path_to_corpus = environ["FENGSHUI_PATH"] + "/corpus/Sa/"

class staticAnalyzer(object):
    def __init__(self, prefix, llvm_link_target, 
                module_name, max_step, build_target, dep_module, callgraph_mode):
        self._prefix = prefix
        self._llvm_link_target = llvm_link_target
        self._module_name = module_name
        self._max_step = max_step
        self._build_target = build_target
        self._dep_module = dep_module
        self._callgraph_mode = callgraph_mode

        self._path_to_corpus = join(path_to_corpus, self._module_name)
        self._path_to_KI = join(self._path_to_corpus, "KI")
        self._path_to_TO = join(self._path_to_corpus, "TO")
        self._path_to_CG = join(self._path_to_corpus, "CG")
        self._path_to_alloc = join(self._path_to_corpus, "alloc")
        self._path_to_deref = join(self._path_to_corpus, "deref")
        self._path_to_free = join(self._path_to_corpus, "free")
        self._path_to_type1 = join(self._path_to_corpus, "type1")
        self.setup_env()

    def setup_env(self):
        print("create %s" % self._path_to_corpus)
        if not exists(self._path_to_corpus):
            mkdir(self._path_to_corpus)

        print("create %s" % self._path_to_KI)
        if not exists(self._path_to_KI):
            mkdir(self._path_to_KI)

        print("create %s" % self._path_to_TO)
        if not exists(self._path_to_TO):
            mkdir(self._path_to_TO)

        print("create %s" % self._path_to_CG)
        if not exists(self._path_to_CG):
            mkdir(self._path_to_CG)

        print("create %s" % self._path_to_alloc)
        if not exists(self._path_to_alloc):
            mkdir(self._path_to_alloc)

        print("create %s" % self._path_to_deref)
        if not exists(self._path_to_deref):
            mkdir(self._path_to_deref)

        print("create %s" % self._path_to_free)
        if not exists(self._path_to_free):
            mkdir(self._path_to_free)

        print("create %s" % self._path_to_type1)
        if not exists(self._path_to_type1):
            mkdir(self._path_to_type1)

    def run(self):
        """
        print("### Generating bitcode for module %s" % str(self._module_name))
        bitcodegen = bcgenerator.bitcodeGenerator(
                            self._build_target, 
                            self._prefix,
                            join(self._path_to_KI, self._llvm_link_target))
        bitcodegen.build()
        print("### Done")

        print("### Parsing objects in module %s" % str(self._module_name))
        parser = obparser.objParser(
                            self._module_name, 
                            self._prefix,
                            join(self._path_to_KI, self._llvm_link_target), 
                            self._build_target, 
                            self._max_step, 
                            self._path_to_TO)
        parser.run()
        print("### Done")
        """

        print("### Generating call graph for module %s" % str(self._module_name))
        if self._callgraph_mode == "semantic":
            callgraph = cg.callGraph(
                            self._module_name, 
                            join(self._path_to_KI, self._llvm_link_target),
                            self._dep_module,
                            self._path_to_CG)
            callgraph.construct()
            callgraph.generate_sew_info()
            #callgraph.load_callgraph()
            callgraph.delete_init_text_func()
            callgraph.sew_up()
            callgraph.dump_rev_call_graph(join(self._path_to_CG, "final.dot"))
        elif self._callgraph_mode == "type": 
            callgraph = cg.callGraph(
                            self._module_name,
                            join(self._path_to_KI, self._llvm_link_target),
                            self._dep_module,
                            "/home/yueqi/fengshui/corpus/Sa/CGC/allnoconfig_plus_added_module/type_match"
                            )
            callgraph.load_callgraph_prototype()
        elif self._callgraph_mode == "kint":
            callgraph = cg.callGraph(
                            self._module_name,
                            join(self._path_to_KI, self._llvm_link_target),
                            self._dep_module,
                            "/home/yueqi/fengshui/revision/benchmarks/defconfig/kint/"
                            )
            callgraph.load_kint_callgraph()
        elif self._callgraph_mode == "kint2":
            callgraph = cg.callGraph(
                            self._module_name,
                            join(self._path_to_KI, self._llvm_link_target),
                            self._dep_module,
                            "/home/yueqi/fengshui/revision/benchmarks/allnoconfig+/kint2/"
                            )
            callgraph.load_kint_callgraph()
        print("### Done")
        
        print("### Generating allocation guidance in module %s" % str(self._module_name))
        allocguider = alloc.allocGuider(
                            self._module_name,
                            self._max_step,
                            callgraph,
                            self._path_to_TO,
                            self._path_to_alloc,
                            self._callgraph_mode)
        allocguider.run()
        print("### Done")

        print("### Generating free guidance in module %s" % str(self._module_name))
        freeguider = free.freeGuider(
                            self._module_name,
                            self._max_step,
                            self._prefix,
                            self._build_target,
                            callgraph,
                            self._path_to_TO,
                            self._path_to_free,
                            False,
                            self._callgraph_mode)
        freeguider.run()
        print("### Done")

        print("### Generating dereference guidance in module %s" % str(self._module_name))
        derefguider = deref.derefGuider(
                            self._module_name,
                            self._max_step,
                            self._prefix,
                            self._build_target,
                            callgraph,
                            self._path_to_TO,
                            self._path_to_deref,
                            False,
                            self._callgraph_mode)
        derefguider.run()
        print("### Done")

        print("### Generating type1 guidance in module %s" % str(self._module_name))
        type1guider = type1.type1Guider(
                            self._module_name,
                            callgraph,
                            self._path_to_TO,
                            self._path_to_type1,
                            self._callgraph_mode)
        type1guider.run()
        print("### Done")

        print("### Generating fzz hint for module %s" % str(self._module_name))
        hinter = fzzhinter.fzzHinter(
                            self._path_to_corpus, 
                            self._callgraph_mode)
        hinter.run()
        print("### Done")
        return
