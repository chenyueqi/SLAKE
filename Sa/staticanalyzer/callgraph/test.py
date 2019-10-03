#!/usr/bin/python
from __future__ import print_function
from os import mkdir
from os.path import exists, join
import shutil
import callgraph

def test():
    call_graph = callgraph.callGraph()

    """
    module_name = "CONFIG_SECURITY_SELINUX"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_SECURITY_SELINUX/selinux-mod.ll"
    call_graph.construct(module_name, bitcode_file)
    return

    module_name = "CONFIG_EXT4_FS"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_EXT4_FS/ext4-mod.ll"
    call_graph.construct(module_name, bitcode_file)
    return

    module_name = "CONFIG_BLOCK"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_BLOCK/block-mod.ll"
    call_graph.construct(module_name, bitcode_file)
    return

    module_name = "CONFIG_SND_SEQUENCER"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_SND_SEQUENCER/snd-seq.ll"
    call_graph.construct(module_name, bitcode_file)
    return

    module_name = "CONFIG_SND_TIMER"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_SND_TIMER/timer.ll"
    call_graph.construct(module_name, bitcode_file)

    module_name = "CONFIG_JBD2"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_JBD2/jbd2-mod.ll"
    call_graph.construct(module_name, bitcode_file)
    return
    """
    """

    module_name = "CONFIG_USB_MON"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_USB_MON/usbmon-mod.ll"
    call_graph.construct(module_name, bitcode_file)

    module_name = "CONFIG_KEYS"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_KEYS/keys-mod.ll"
    call_graph.construct(module_name, bitcode_file)

    module_name = "CONFIG_IP_MROUTE"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_IP_MROUTE/ipmr.ll"
    call_graph.construct(module_name, bitcode_file)

    module_name = "CONFIG_AIO"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_AIO/aio.ll"
    call_graph.construct(module_name, bitcode_file)

    module_name = "CONFIG_TTY"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_TTY/tty-mod.ll"
    call_graph.construct(module_name, bitcode_file)

    call_graph = callgraph.callGraph()
    module_name = "allnoconfig"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/allnoconfig/vmlinux.bc"
    call_graph.construct(module_name, bitcode_file)
    return 
    """
    """
    module_name = "CONFIG_SND_TIMER"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_SND_TIMER/timer.ll"
    call_graph.construct(module_name, bitcode_file)

    module_name = "CONFIG_SND_HRTIMER"
    bitcode_file = "/home/yueqi/fengshui/dev/SA/Corpus/KIC/CONFIG_SND_HRTIMER/hrtimer.ll"
    call_graph.construct(module_name, bitcode_file)

    # graph_path = "/home/yueqi/fengshui/dev/SA/Corpus/CGC/alldefconfig/callgraph_noinitext.dot"
    graph_path = "/home/yueqi/fengshui/dev/SA/Corpus/CGC/CONFIG_SECURITY_SELINUX/callgraph_final.dot"
    call_graph.load_callgraph(graph_path)
    start_func = ["sel_netif_sid_slow"]
    reachable = call_graph.backward_reachable_func(start_func)
    print(reachable)
    """
    """
    graph_path1 = "/home/yueqi/fengshui/dev/SA/Corpus/CGC/CONFIG_USB_MON/callgraph_final.dot"
    graph_path2 = "/home/yueqi/fengshui/dev/SA/Corpus/CGC/allnoconfig/callgraph_final.dot"

    call_graph1 = callgraph.callGraph()
    call_graph1.load_callgraph(graph_path1)

    call_graph2 = callgraph.callGraph()
    call_graph2.load_callgraph(graph_path2)

    call_graph3 = callgraph.callGraph()
    call_graph3.merge_rev_call_graph(call_graph1, call_graph2, "CONFIG_USB_MON")
    """
    graph_path = "/home/yueqi/fengshui/dev/SA/Corpus/CGC/alldefconfig/callgraph_final.dot"
    call_graph.load_callgraph(graph_path)
    start_func = ["avc_node_delete", "avc_node_replace"]
    reachable = call_graph.backward_reachable_func(start_func)
    print(reachable)

if __name__ == '__main__':
    test()
