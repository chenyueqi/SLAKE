add_llvm_loadable_module( LLVMAllocSite
  AllocSite.cpp
  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
