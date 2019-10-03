#!/usr/bin/python
from __future__ import print_function
from os import mkdir
from os.path import exists, join
import shutil
import bitcodegenerator

def test():
    cc = "clang"
    cflags = "-O0 -fno-inline-functions"

    """
    target = ["net/core/fib_rules.ll"]
    target = ["security/selinux/netlabel.ll"]
    target = ["fs/timerfd.ll"]
    target = ["fs/posix_acl.ll"]
    target = ["kernel/pid_namespace.ll"]
    target = ["lib/assoc_array.ll"]
    target = ["kernel/seccomp.ll"]
    """
    # prefix = "sound/core/seq/"
    # target = "seq.o seq_lock.o seq_clientmgr.o seq_memory.o seq_queue.o seq_fifo.o seq_prioq.o seq_timer.o seq_system.o seq_ports.o"

    prefix = "fs/ext4/"
    target = "balloc.o bitmap.o block_validity.o dir.o ext4_jbd2.o extents.o extents_status.o file.o fsmap.o fsync.o hash.o ialloc.o indirect.o inline.o inode.o ioctl.o mballoc.o migrate.o mmp.o move_extent.o namei.o page-io.o readpage.o resize.o super.o symlink.o sysfs.o xattr.o xattr_trusted.o xattr_user.o"
    gen = bitcodegenerator.bitcodeGenerator(cc, cflags, target, prefix)
    gen.build()
    gen.link("ext4-mod.ll")

if __name__ == '__main__':
    test()
