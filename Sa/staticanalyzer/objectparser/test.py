#!/usr/bin/python
from __future__ import print_function
from os import mkdir, environ
from os.path import exists, join
import shutil
import objparser
import sys
sys.path.append("../callgraph/")
import callgraph as cg
import candiobjparser

path_to_bitcode_kernel = environ["FENGSHUI_PATH"] + "/corpus/Sa/KIC/clang_kernel"

def test():
    prefix = "sound/core"
    module_name = "CONFIG_SND_HRTIMER"
    bitcode_file = "hrtimer.ll" 
    max_step = 3
    build_target = "hrtimer.o"
    parser = objparser.objParser(module_name, prefix, bitcode_file, build_target, max_step)
    parser.run()

    """
    module_name = "CONFIG_NETLABEL"
    bitcode_file = join(path_to_bitcode_kernel, "security/selinux/netlabel.ll")
    max_step = 3
    build_target = "security/selinux/netlabel.o"

    module_name = "CONFIG_TIMERFD"
    bitcode_file = join(path_to_bitcode_kernel, "fs/timerfd.ll")
    max_step = 3
    build_target = "fs/timerfd.o"

    module_name = "CONFIG_FS_POSIX_ACL"
    bitcode_file = join(path_to_bitcode_kernel, "fs/posix_acl.ll")
    max_step = 3
    build_target = "fs/posix_acl.o"

    module_name = "CONFIG_PID_NS"
    bitcode_file = join(path_to_bitcode_kernel, "kernel/pid_namespace.ll")
    max_step = 3
    build_target = "kernel/pid_namespace.o"
    
    module_name = "CONFIG_ASSOCIATIVE_ARRAY"
    bitcode_file = join(path_to_bitcode_kernel, "lib/assoc_array.ll")
    max_step = 3
    build_target = "lib/assoc_array.o"

    module_name = "CONFIG_SECCOMP"
    bitcode_file = join(path_to_bitcode_kernel, "kernel/seccomp.ll")
    max_step = 3
    build_target = "kernel/seccomp.o"

    module_name = "CONFIG_SND_SEQUENCER"
    bitcode_file = join(path_to_bitcode_kernel, "sound/core/seq/snd-seq.ll")
    max_step = 3
    build_target = "seq.o seq_lock.o seq_clientmgr.o seq_memory.o seq_queue.o seq_fifo.o seq_prioq.o seq_timer.o seq_system.o seq_ports.o"

    module_name = "CONFIG_EXT4_FS"
    # bitcode_file = join(path_to_bitcode_kernel, "fs/ext4/ext4-mod.ll")
    max_step = 3
    prefix = "fs/ext4/"
    bitcode_file = "ext4-mod.ll"
    build_target = "balloc.o bitmap.o block_validity.o dir.o ext4_jbd2.o extents.o extents_status.o file.o fsmap.o fsync.o hash.o ialloc.o indirect.o inline.o inode.o ioctl.o mballoc.o migrate.o mmp.o move_extent.o namei.o page-io.o readpage.o resize.o super.o symlink.o sysfs.o xattr.o xattr_trusted.o xattr_user.o"
    """

    """
    """
    """
    module_name = "CONFIG_JBD2"
    max_step = 3
    callgraph = cg.callGraph()
    graph_path = "/home/yueqi/fengshui/dev/SA/Corpus/CGC/CONFIG_JBD2/callgraph_final.dot"
    callgraph.load_callgraph(graph_path)
    parser = callgraphparser.callgraphParser(module_name, callgraph, max_step)
    parser.run()
    """

if __name__ == '__main__':
    test()
