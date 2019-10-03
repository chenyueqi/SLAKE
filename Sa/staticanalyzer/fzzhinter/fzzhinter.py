#!/usr/bin/python
from __future__ import print_function
from os import listdir
from os import mkdir
from os.path import exists, join, isfile
from shutil import rmtree

privilege_syscall = [
# CAP_SYS_BOOT
"SyS_reboot", "SyS_kexec_load",
# CAP_SYS_ADMIN
"SyS_swapon", "SyS_swapoff", "SyS_umount", "SyS_oldumount", "SyS_quotactl",
"SyS_mount", "SyS_pivot_root", "SyS_lookup_dcookie", "SyS_bdflush",
# CAP_SYS_MODULE
"SyS_finit_module", "SyS_init_module", "SyS_delete_module",
# CAP_DAC_READ_SEARCH
"SyS_open_by_handle_at",
# CAP_CHOWN
"SyS_fchown", "SyS_fchown16", "SyS_fchownat", "SyS_lchown", "SyS_lchown16",
"SyS_chown", "SyS_chown16", "SyS_fchmodat", "SyS_chmod", "SyS_fchmod",
# CAP_SYS_PACCT
"SyS_acct",
# CAP_SYS_TIME
"SyS_settimeofday", "SyS_stime", "SyS_adjtimex",
# CAP_SYS_CHROOT
"SyS_chroot",
# CAP_SYSLOG
"SyS_syslog"
]

class fzzHinter(object):
    def __init__(self, path_to_corpus="", callgraph_mode = "semantic"):
        if path_to_corpus=="":
            print("Please specify corpus path")
            return
        self._path_to_corpus = path_to_corpus
        self._callgraph_mode = callgraph_mode

    def run(self):
        if exists(join(self._path_to_corpus, "alloc")):
            self.get_hint(join(self._path_to_corpus, "alloc"))
        if exists(join(self._path_to_corpus, "deref")):
            self.get_hint(join(self._path_to_corpus, "deref"))
        if exists(join(self._path_to_corpus, "free")):
            self.get_hint(join(self._path_to_corpus, "free"))
        if exists(join(self._path_to_corpus, "type1")):
            self.get_hint(join(self._path_to_corpus, "type1"))

    def get_hint(self, path_to_workdir):
        if self._callgraph_mode == "semantic":
            path_to_end = join(path_to_workdir, "end_semantic")
            path_to_hint = join(path_to_workdir, "hint_semantic")
        elif self._callgraph_mode == "type":
            path_to_end = join(path_to_workdir, "end_plain")
            path_to_hint = join(path_to_workdir, "hint_plain")
        elif self._callgraph_mode == "kint":
            path_to_end = join(path_to_workdir, "end_kint")
            path_to_hint = join(path_to_workdir, "hint_kint")
        elif self._callgraph_mode == "kint2":
            path_to_end = join(path_to_workdir, "end_kint2")
            path_to_hint = join(path_to_workdir, "hint_kint2")
            
        if exists(path_to_hint):
            rmtree(path_to_hint)
        mkdir(path_to_hint)
        files = [f for f in listdir(path_to_end) if isfile(join(path_to_end, f))]
        for f in files:
            content = open(join(path_to_end, f)).read().split('\n')
            sys_entry = []
            for func in content:
                if func in privilege_syscall:
                    continue
                if "SyS_" in func and "compat_SyS_" not in func:
                    sys_entry.append(func)
            if sys_entry == []:
                continue
            hint_file = file(join(path_to_hint, f), 'w')
            for entry in sys_entry:
                print(entry, file=hint_file)
