#!/usr/bin/python
from __future__ import print_function
from os import chdir
from os import mkdir
from os import environ
from os import listdir
from os.path import exists
from os.path import join
from os.path import isfile
import shutil
import subprocess
import sys

build_util = environ["FENGSHUI_PATH"] + "/src/Sa/utils/build_call_graph.sh"
init_text_func_util = environ["FENGSHUI_PATH"] + "/src/Sa/utils/obtain_init_text_func.sh"
sew_util = environ["FENGSHUI_PATH"] + "/src/Sa/utils/sew_call_graph.sh"
path_to_corpus = environ["FENGSHUI_PATH"] + "/corpus/Sa/"

class callGraph(object):
    def __init__(self, module_name, bitcode_file, dep_module, path_to_CG):
        self._rev_call_graph = {} # dst -> list of src in format of name
        self._module_name = module_name
        self._bitcode_file = bitcode_file
        self._dep_module = dep_module
        self._path_to_CG = path_to_CG
        self._callgraph_path = join(self._path_to_CG, "callgraph_final.dot")
        self._path_to_in = join(self._path_to_CG, "in/")
        self._path_to_out = join(self._path_to_CG, "out/")
        self._edge_num = 0
        self._node_num = 0

    def construct(self):
        print("write callgraph_final.dot to %s" % self._path_to_CG)
        chdir(self._path_to_CG)
        cmd = [build_util, self._bitcode_file]
        subprocess.call(cmd)

        self.delete_init_text_func()
        self.load_callgraph()

    def delete_init_text_func(self):
        dump_file = join(self._path_to_CG, "init_text_func")
        print("collect function in .init.text section")
        cmd = [init_text_func_util, self._bitcode_file, str(dump_file)]
        subprocess.call(cmd)
        print("delete init text function from callgraph_final.dot")
        init_text_func = []
        funcs = open(dump_file, 'r').read().split('\n')
        for func in funcs:
            if func == "":
                continue
            init_text_func.append(func)
        init_text_node = []
        callgraph = open(self._callgraph_path, 'r').read().split('\n')
        for log in callgraph:
            if log == "" or log == "}" or log == '\t' or "Call Graph" in log:
                continue
            if "shape=" in log:
                func_name = log.split('{')[1].split('}')[0]
                if func_name in init_text_func:
                    node_id = log.split(' [')[0].split('\t')[1]
                    init_text_node.append(node_id)
        new_call_graph_file = open(self._callgraph_path, 'w')
        for log in callgraph:
            if log == "":
                continue
            if log == "}" or log == "\t" or "Call Graph" in log:
                print(log, file=new_call_graph_file)
                continue
            if "shape=" in log:
                node_id = log.split(' [')[0].split('\t')[1]
                if node_id in init_text_node:
                    continue
                else:
                    print(log, file=new_call_graph_file)
            else:
                caller = log.split(' ->')[0].split('\t')[1]
                callee = log.split('-> ')[1].split('[')[0]
                if caller in init_text_node or callee in init_text_node:
                    continue
                else:
                    print(log, file=new_call_graph_file)
        
    def load_callgraph(self):
        if not exists(self._callgraph_path):
            print("%s doesn't exist" % self._callgraph_path)
            return
        callgraph = open(self._callgraph_path, 'r').read().split('\n')
        self._node_num = 0
        self._edge_num = 0
        node_dict = {}
        node_list = {}
        for log in callgraph:
            if log == "" or log == "}" or log ==  '\t' or "Call Graph" in log:
                continue
            if "shape=" in log:
                self._node_num = self._node_num + 1
                func_name = log.split('{')[1].split('}')[0]
                node_id  = log.split(' [')[0].split('\t')[1]
                node_dict[node_id] = func_name
            else:
                src = log.split(' ->')[0].split('\t')[1]
                dst = log.split('-> ')[1].split('[')[0]
                if node_list.has_key(dst) == False:
                    node_list[dst] = []
                if src not in node_list[dst]:
                    self._edge_num = self._edge_num + 1
                    node_list[dst].append(src)
        print("# of node:%u" % self._node_num)
        print("# of edge:%u" % self._edge_num)
        for node_id in node_dict:
            func_name = node_dict[node_id]
            self._rev_call_graph[func_name] = []
        for dst in node_list:
            dst_name = node_dict[dst]
            for src in node_list[dst]:
                src_name = node_dict[src]
                self._rev_call_graph[dst_name].append(src_name)

    def load_callgraph_prototype(self):
        if not exists(self._callgraph_path):
            print("%s doesn't exist" % self._callgraph_path)
            return
        print("[+] loading call graph from %s" % self._callgraph_path)
        self._edge_num = 0
        self._node_num = 0
        callgraph = open(self._callgraph_path, 'r').read().split('\n')
        node_dict = {}
        node_list = {}
        for log in callgraph:
            if log == "" or log == "}" or log ==  '\t' or "Call graph" in log:
                continue
            if "shape=" in log:
                self._node_num = self._node_num + 1
                func_name = log.split('{')[1].split('}')[0]
                node_id  = log.split(' [')[0].split('\t')[1]
                node_dict[node_id] = func_name
            else:
                src = log.split(' ->')[0].split('\t')[1]
                dst = log.split('-> ')[1].split(';')[0]
                if node_list.has_key(dst) == False:
                    node_list[dst] = []
                if src not in node_list[dst]:
                    self._edge_num = self._edge_num + 1
                    node_list[dst].append(src)
        print("# of node:%u" % self._node_num)
        print("# of edge:%u" % self._edge_num)
        for node_id in node_dict:
            func_name = node_dict[node_id]
            self._rev_call_graph[func_name] = []
        for dst in node_list:
            if dst not in node_dict:
                continue
            dst_name = node_dict[dst]
            for src in node_list[dst]:
                src_name = node_dict[src]
                self._rev_call_graph[dst_name].append(src_name)

    def load_kint_callgraph(self):
        if not exists(self._callgraph_path):
            print("%s doesn't exist" % self._callgraph_path)
            return
        print("[+] loading call graph from %s" % self._callgraph_path)
        self._edge_num = 0
        self._node_num = 0
        callgraph = open(self._callgraph_path, 'r').read().split('\n')
        for log in callgraph:
            if "Num of Callees" in log or log == "":
                continue
            src = log.split(';')[0].split(":")[1]
            dsts = log.split(';')[1].split(":")
            for dst in dsts:
                if dst == "":
                    continue
                if self._rev_call_graph.has_key(dst) == False:
                    self._rev_call_graph[dst] = []
                if src not in self._rev_call_graph[dst]:
                    self._edge_num = self._edge_num + 1
                    self._rev_call_graph[dst].append(src)
        print("# of node:unknown")
        print("# of edge:%u" % self._edge_num)

    def generate_sew_info(self):
        self.setup_sew_env()
        print("write sew info to %s" % self._path_to_CG)
        cmd = [sew_util, self._bitcode_file, self._path_to_in, join(self._path_to_out, "corpus")]
        subprocess.call(cmd)

    def setup_sew_env(self):
        print("create %s" % self._path_to_in)
        if exists(self._path_to_in):
            shutil.rmtree(self._path_to_in)
        mkdir(self._path_to_in)

        print("create %s" % self._path_to_out)
        if exists(self._path_to_out):
            shutil.rmtree(self._path_to_out)
        mkdir(self._path_to_out)

    def sew_up(self):
        print("sewing up with")
        for m in self._dep_module:
            print("Module: %s" % str(m))
            self.sew_up_module(m)
        print("total # of node:%u" % self._node_num)
        print("total # of edge:%u" % self._edge_num)

    def sew_up_module(self, module):
        path_to_module_CG = join(path_to_corpus, module, "CG")
        if not exists(path_to_module_CG):
            print("ERR module %s doesn't exist" % module)
            return
        module_callgraph = callGraph(module, "", [], path_to_module_CG)
        module_callgraph.load_callgraph()
        for callee in module_callgraph._rev_call_graph:
            if self._rev_call_graph.has_key(callee) == False:
                self._node_num = self._node_num + 1
                self._rev_call_graph[callee] = []
            for caller in module_callgraph._rev_call_graph[callee]:
                if caller not in self._rev_call_graph[callee]:
                    self._edge_num = self._edge_num + 1
                    self._rev_call_graph[callee].append(caller)
        self.link_out_in(self._path_to_out, join(path_to_module_CG, "in"))
        self.link_out_in(join(path_to_module_CG, "out"), self._path_to_in)

    def link_out_in(self, path_to_out, path_to_in):
        out_dict = {} # struct_name:offset -> caller func
        if not exists(join(path_to_out, "corpus")):
            print("%s doesn't exist" % str(join(path_to_out, "corpus")))
            return
        content = open(join(path_to_out, "corpus")).read().split('\n')
        for t in content:
            if t == "":
                continue
            struct_name = t.split(':')[0].split('.')[1]
            offset = t.split(':')[1]
            caller = t.split(':')[2]
            key = struct_name+":"+offset
            if out_dict.has_key(key) == False:
                out_dict[key] = []
            if caller not in out_dict[key]:
                out_dict[key].append(caller)

        in_dict = {}
        files = [f for f in listdir(path_to_in) if isfile(join(path_to_in, f))]
        for f in files:
            struct_name = f.split(':')[0].split('.')[1]
            content = open(join(path_to_in, f)).read().split('\n')
            for t in content:
                if t == "":
                    continue
                offset = t.split(':')[0]
                callee = t.split(':')[1]
                key = struct_name+":"+offset
                if in_dict.has_key(key) == False:
                    in_dict[key] = []
                if callee not in in_dict[key]:
                    in_dict[key].append(callee)
                
        for key in in_dict:
            if not out_dict.has_key(key):
                continue
            for callee in in_dict[key]:
                if self._rev_call_graph.has_key(callee) == False:
                    self._node_num = self._node_num + 1
                    self._rev_call_graph[callee] = []
                for caller in out_dict[key]:
                    if caller not in self._rev_call_graph[callee]:
                        self._edge_num = self._edge_num + 1
                        self._rev_call_graph[callee].append(caller)

    def backward_reachable_func(self, start_func):
        if self._rev_call_graph == {}:
            print("Please first load call graph")
            return []
        if start_func == []:
            print("Please provide at lease one start_func")
            return []

        queue = []
        reachable = []
        print("[+] Reachable analysis from %s" % str(start_func))
        for func in start_func:
            if func in self._rev_call_graph:
                queue.append(func)
        while len(queue) != 0: #dfs
            callee = queue.pop()
            if callee in reachable:
                continue
            else:
                reachable.append(callee)
            if callee not in self._rev_call_graph:
                continue
            for caller in self._rev_call_graph[callee]:
                if caller not in reachable:
                    queue.append(caller)
        return reachable

    def dump_rev_call_graph(self, path_to_dump):
        print("dump reverse call graph to %s" % path_to_dump)
        dump_file = file(path_to_dump, 'w')
        for callee in self._rev_call_graph:
            print("%s:%s" % (str(callee), str(self._rev_call_graph[callee])), file=dump_file)
