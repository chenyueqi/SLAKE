#!/usr/bin/python
from __future__ import print_function
from os import mkdir
from os import environ
from os.path import exists
from os.path import join
import shutil
import type1objparser

path_to_corpus = environ["FENGSHUI_PATH"] + "/corpus/Sa"

def test():
    build_target = ["fs/eventpoll.o"]
    module_name = "CONFIG_EPOLL"
    parser = type1objparser.type1ObjParser(build_target, join(path_to_corpus, module_name, "TO/type1"))
    parser.run()

if __name__ == '__main__':
    test()
