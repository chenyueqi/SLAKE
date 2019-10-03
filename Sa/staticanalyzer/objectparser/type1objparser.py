from __future__ import print_function
from os import mkdir
from os import environ
from os import chdir
from os.path import join
import subprocess

path_to_kernel=environ["FENGSHUI_PATH"] + "/vm/kernel/"
path_to_plugin=path_to_kernel+"scripts/gcc-plugins/fengshui_plugin.c"
path_to_plugin_h=path_to_kernel+"scripts/gcc-plugins/fengshui_plugin.h"

class type1ObjParser(object):
    """ Using GCC PLugin to find site invoking copy_from_user in kernel
    """
    
    def __init__(self, build_targets, path_to_type1):
        """
        Args:
            build_targets: a list of targets to be build
            path_to_type1: path to store results
        """
        self._build_targets = build_targets
        self.gcc_result_path = path_to_type1 + "/gcc"
        self.modify_gcc_plugin()

    def modify_gcc_plugin(self):
        plugin_file = file(path_to_plugin, 'r').read().split('\n')
        for i in range(len(plugin_file)-1):
            if "static char* dump_file_path =" in plugin_file[i]:
                plugin_file[i] = ("static char* dump_file_path = (char*)\"" +
                                    self.gcc_result_path +  "\";")
                print("write gcc result to %s" % self.gcc_result_path)
                break
        new_plugin_file = open(path_to_plugin, 'w')
        for i in range(len(plugin_file)-1):
            print(plugin_file[i], file=new_plugin_file)

        plugin_file_h = open(path_to_plugin_h, 'w')
        print("#define STATICANA", file = plugin_file_h)
        print("#define TYPE1_PLUGIN", file = plugin_file_h)

    def run(self):
		self.build_kernel_target()

    def build_kernel_target(self):
        self.make_clean()
        for target in self._build_targets:
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
