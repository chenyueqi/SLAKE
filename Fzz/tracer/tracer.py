#!/usr/bin/python
from __future__ import print_function
from os import mkdir, listdir, remove
from os.path import exists, join, isfile
import sys
import subprocess
import os
import shutil

path_to_testcast = "/home/yueqi/fengshui/testcases/"
path_to_trace = "/home/yueqi/fengshui/trace"

class layout_record:
    def __init__(self, callsite, ptr):
        self._callsite = callsite
        self._ptr = ptr

class Tracer(object):
    def __init__(self, struct_name, callsite):
        self._struct_name = struct_name
        self._callsite = callsite
        self._layout = []
        self._trace_records = []
        self._start = 0
        self._finish = 0
        self._bytes_alloc = 0
        self._alloc_ptr = 0
        self._corpus_path = path_to_testcast + self._struct_name + "/alloc/"
        self._trace_path = join(self._corpus_path, "trace/")
        self.setup_env()

    def setup_env(self):
		if not os.path.exists(self._trace_path):
			os.mkdir(self._trace_path)
		shutil.copy2(path_to_trace, self._trace_path)
		callsite_file = file(join(self._trace_path, "callsite"), 'w')
		print(str(self._callsite), file=callsite_file)

    def analyze(self):
        self.find_start_end()
        self.get_layout()

    def find_start_end(self):
				print("begin analysis, call site: 0x%s" % str(self._callsite))
				self._trace_records = file(join(self._trace_path, "trace"), 'r').read().split('\n')
				alloc = 0
				self._start = 0
				# self._finish = len(self._trace_records)
				self._finish = 0
				for i in range(len(self._trace_records)):
					if "min_prog0_trace" in self._trace_records[i] and \
                self._callsite in self._trace_records[i]:
						alloc = i
						start_syscall = 0
						finish_syscall = 0
						# pin start of "allocation" syscall
						for j in range(alloc, 0, -1):
							if "min_prog0_trace" in self._trace_records[j] and \
									"start calling syscall number" in self._trace_records[j]:
								self._start = j
								start_syscall = self._trace_records[j].split("tart calling syscall number: ")[1]
								break
						# pin finish of "allocation" syscall
						for j in range(alloc, len(self._trace_records)):
							if "min_prog0_trace" in self._trace_records[j] and \
									"finish calling syscall number" in self._trace_records[j]:
								self._finish = j
								finish_syscall = self._trace_records[j].split("finish calling syscall number: ")[1]
								break
						if start_syscall != finish_syscall:
							print("mismatch between start syscall and finish syscall")
							continue
						else:
							break

				print("syscall range:", self._start + 1, alloc + 1, self._finish + 1)
				self._bytes_alloc = self._trace_records[alloc].split("bytes_alloc=")[1].split(' ')[0]
				self._alloc_ptr = self._trace_records[alloc].split("ptr=")[1].split(' ')[0]
				print("bytes_alloc:", self._bytes_alloc)
				print("ptr:", self._alloc_ptr)

    def get_layout(self):
        print("getting layout from", self._start + 1, "to", self._finish + 1)
        self._layout = []
        for i in range(self._start, self._finish):
            if "free=" in self._trace_records[i]:
                one_ptr = self._trace_records[i].split("ptr=")[1].split(' ')[0]
                for slot_idx in len(self._layout):
                    if self._layout[slot_idx]._ptr == one_ptr:
                        self._layout[slot_idx]._ptr = '0'
                        self._layout[slot_idx]._callsite = '0'
                continue
            if "bytes_alloc=" + self._bytes_alloc in self._trace_records[i]: # and 
                # "kmem_cache_alloc" not in self._trace_records[i]:
                one_ptr = self._trace_records[i].split("ptr=")[1].split(' ')[0]
                call_site = self._trace_records[i].split("call_site=")[1].split(' ')[0]
                fill_flag = False
                for slot in self._layout:
                    if slot._callsite == '0' and slot._ptr == '0':
                        slot._callsite = call_site
                        slot._ptr = one_ptr
                        fill_flag = True
                        break
                if fill_flag:
                    continue
                slot = layout_record(call_site, one_ptr)
                self._layout.append(slot)
                continue

        for i in range(len(self._layout)):
            if self._layout[i]._ptr == self._alloc_ptr:
                print("2")
                continue
            if self._layout[i]._ptr == '0':
                print("0")
                continue
            print("1")


