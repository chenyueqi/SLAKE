#!/usr/bin/python
from __future__ import print_function
from os import mkdir
from os.path import exists, join
import shutil
import tracer

def test():
    # worker = tracer.Tracer("key_construction", "ffffffff812cf16b")
    # worker = tracer.Tracer("key_restriction", "ffffffff812cbc03")
    # worker = tracer.Tracer("pipe_buffer", "ffffffff811965ec")
    # worker = tracer.Tracer("ext4_ext_path", "ffffffff8120762e")
    # worker = tracer.Tracer("mnt_namespace", "ffffffff811ac50a")
    # worker = tracer.Tracer("snd_seq_queue", "ffffffff816f0934")
    # worker = tracer.Tracer("eventpoll", "ffffffff811ce648")
    # worker = tracer.Tracer("pipe_inode_info", "ffffffff81195ad5")
    # worker = tracer.Tracer("file_lock", "ffffffff811d5d72")
    # worker = tracer.Tracer("mount", "ffffffff811acf2a")
    # worker = tracer.Tracer("uts_namespace", "ffffffff810e1f79")
    # worker = tracer.Tracer("listeners", "ffffffff8175a62c")
    # worker = tracer.Tracer("snd_seq_timer", "ffffffff816f2177")
    worker = tracer.Tracer("vmap_area", "ffffffff811cddef")
    worker.analyze()

if __name__ == '__main__':
    test()
