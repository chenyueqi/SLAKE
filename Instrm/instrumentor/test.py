#!/usr/bin/python
from __future__ import print_function
import instrumentor
import sys

if __name__ == '__main__':
    if len(sys.argv) == 2:
        flags = ""
        obj_name = sys.argv[1]
    elif len(sys.argv) == 3:
        flags = sys.argv[1]
        obj_name = sys.argv[2]
    """
    flags = "DEREF"
    obj_name = "eppoll_entry"
    path_to_candi = "/home/yueqi/fengshui/corpus/Sa/CONFIG_EPOLL/TO/candi/3"
    """
    """
    flags = "RCU"
    obj_name = "vmap_area"
    """
    """
    flags = "FREE"
    obj_name = "key_restriction"
    """
    """
    flags = ""
    obj_name = "pm_qos_request"
    """
    instm = instrumentor.insTrumentor(flags, obj_name, "")
    instm.run()
