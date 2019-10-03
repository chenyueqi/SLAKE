#!/usr/bin/python
from __future__ import print_function
from __future__ import division
from os import mkdir
from os import environ
from os import listdir
from os.path import exists
from os.path import join
from os.path import isfile
import shutil
import staticanalyzer

path_to_corpus = environ["FENGSHUI_PATH"] + "/corpus/Sa/"
path_to_configuration = environ["FENGSHUI_PATH"] + "/src/Sa/staticanalyzer/config/"

prefix = ""
llvm_link_target = ""
module_name = ""
build_target = ""
dep_module = []

callgraph_mode = "kint2"

def help_assemble(sub_prefix, sub_build_target, build_target):
    sub_build_target_list = sub_build_target.split(' ')
    for targets in sub_build_target_list:
        new_target = sub_prefix + targets
        build_target = build_target + " " + str(new_target)
    return build_target

def load_configuration(module_name):
    global prefix
    global llvm_link_target
    global build_target
    global dep_module
    content = open(join(path_to_configuration, module_name)).read().split('\n')
    for config in content:
        if config == "":
            continue
        if "prefix:" in config:
            prefix = config.split(':')[1]
        if "llvm_link_target:" in config:
            llvm_link_target = config.split(':')[1]
        if "build_target:" in config:
            build_target = config.split(':')[1]
        if "dep_module:" in config:
            modules = config.split(':')[1].split(' ')
            for m in modules:
                dep_module.append(str(m))

def dump_configuration():
    global prefix
    global llvm_link_target
    global module_name
    global build_target
    global dep_module
    config_file = open(join(path_to_configuration, module_name), 'w')
    print("prefix:%s" % prefix, file=config_file)
    print("llvm_link_target:%s" % llvm_link_target, file=config_file)
    print("module_name:%s" % module_name, file=config_file)
    print("build_target:%s" % build_target, file=config_file)
    config_file.write("dep_module:")
    for m in dep_module:
        config_file.write(m)
        config_file.write(' ')

def collect_result():
    global module_name
    global callgraph_mode
    path_to_module_corpus = join(path_to_corpus, module_name)
    if callgraph_mode == "type":
        path_to_alloc_hint = join(path_to_module_corpus, "alloc/hint_plain")
        path_to_deref_hint = join(path_to_module_corpus, "deref/hint_plain")
        path_to_free_hint = join(path_to_module_corpus, "free/hint_plain")
        path_to_type1_hint = join(path_to_module_corpus, "type1/hint_plain")
    elif callgraph_mode == "semantic":
        path_to_alloc_hint = join(path_to_module_corpus, "alloc/hint_semantic")
        path_to_deref_hint = join(path_to_module_corpus, "deref/hint_semantic")
        path_to_free_hint = join(path_to_module_corpus, "free/hint_semantic")
        path_to_type1_hint = join(path_to_module_corpus, "type1/hint_semantic")
    elif callgraph_mode == "kint":
        path_to_alloc_hint = join(path_to_module_corpus, "alloc/hint_kint")
        path_to_deref_hint = join(path_to_module_corpus, "deref/hint_kint")
        path_to_free_hint = join(path_to_module_corpus, "free/hint_kint")
        path_to_type1_hint = join(path_to_module_corpus, "type1/hint_kint")
    elif callgraph_mode == "kint2":
        path_to_alloc_hint = join(path_to_module_corpus, "alloc/hint_kint2")
        path_to_deref_hint = join(path_to_module_corpus, "deref/hint_kint2")
        path_to_free_hint = join(path_to_module_corpus, "free/hint_kint2")
        path_to_type1_hint = join(path_to_module_corpus, "type1/hint_kint2")
    else:
        print("[-] Please specify callgraph mode")


    object_num = 0
    files = [f for f in listdir(path_to_alloc_hint) if isfile(join(path_to_alloc_hint, f))]
    object_num = object_num + len(files)
    print("alloc number: %u" % object_num)
    files = [f for f in listdir(path_to_type1_hint) if isfile(join(path_to_type1_hint, f))]
    object_num = object_num + len(files)
    print("object number: %u" % object_num)

    site_num = 0
    syscall_num = 0
    alloc_obj_list = []

    files = [f for f in listdir(path_to_alloc_hint) if isfile(join(path_to_alloc_hint, f))]
    site_num = site_num + len(files)
    for f in files:
        alloc_obj_list.append(f)
        content = open(join(path_to_alloc_hint, f)).read().split('\n')
        syscall_num = syscall_num + len(content) - 1

    files = [f for f in listdir(path_to_deref_hint) if isfile(join(path_to_deref_hint, f))]
    for f in files:
        if f not in alloc_obj_list:
            continue
        site_num = site_num + 1
        content = open(join(path_to_deref_hint, f)).read().split('\n')
        syscall_num = syscall_num + len(content) - 1

    files = [f for f in listdir(path_to_free_hint) if isfile(join(path_to_free_hint, f))]
    for f in files:
        if f not in alloc_obj_list:
            continue
        site_num = site_num + 1
        content = open(join(path_to_free_hint, f)).read().split('\n')
        syscall_num = syscall_num + len(content) - 1

    files = [f for f in listdir(path_to_type1_hint) if isfile(join(path_to_type1_hint, f))]
    site_num = site_num + len(files)
    for f in files:
        content = open(join(path_to_type1_hint, f)).read().split('\n')
        syscall_num = syscall_num + len(content) - 1

    print("site number: %u" % site_num)
    if site_num == 0:
        print("average syscall number: NA")
    else :
        print("average syscall number: %u" % (syscall_num/site_num))

def test(test_module):
    max_step = 3
    global prefix
    global llvm_link_target
    global module_name
    global build_target
    global dep_module
    global callgraph_mode

    module_name = test_module
    load_configuration(module_name)
    analyzer = staticanalyzer.staticAnalyzer(
                                        prefix, 
                                        llvm_link_target,
                                        module_name, 
                                        max_step, 
                                        build_target, 
                                        dep_module, 
                                        callgraph_mode)
    analyzer.run()
    collect_result()

    return

if __name__ == '__main__':
    test("allnoconfig")
    test("CONFIG_PID_NS")
    test("CONFIG_PROC_FS")
    test("CONFIG_SECCOMP")
    test("CONFIG_CGROUPS")
    test("CONFIG_UTS_NS")
    test("CONFIG_AIO")
    test("CONFIG_BLOCK")
    test("CONFIG_EPOLL")
    test("CONFIG_EXT4_FS")
    test("CONFIG_FILE_LOCKING")
    test("CONFIG_FS_POSIX_ACL")
    test("CONFIG_FSNOTIFY")
    test("CONFIG_ISO9660_FS")
    test("CONFIG_FAT_FS")
    test("CONFIG_JBD2")
    test("CONFIG_TIMERFD")
    test("CONFIG_INET")
    test("CONFIG_IP_MROUTE")
    test("CONFIG_IPV6")
    test("CONFIG_NET")
    test("CONFIG_NETLABEL")
    test("CONFIG_POSIX_TIMERS")
    test("CONFIG_SYSVIPC")
    test("CONFIG_TTY")
    test("CONFIG_USB_MON")
    test("CONFIG_SND_HRTIMER")
    test("CONFIG_SND_TIMER")
    test("CONFIG_SND_SEQUENCER")
    test("CONFIG_ASSOCIATIVE_ARRAY")
    test("CONFIG_KEYS")
    test("CONFIG_SECURITY_SELINUX")
