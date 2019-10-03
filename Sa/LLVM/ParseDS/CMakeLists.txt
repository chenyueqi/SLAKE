add_llvm_loadable_module( LLVMParseDS
  ParseDS.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
