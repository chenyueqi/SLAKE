# What this pass do
record kernel functions that invoke slab allocation function for example kmalloc

# How to add this pass to LLVM
```shell
cp ../AllocSite llvm-src/lib/Transforms
```
add
```
add_subdirectory(AllocSite)
```
to llvm-src/lib/Transforms/CMakeLists.txt


# How to build this pass
```shell
cd llvm-src/build
make
```

# How to use this pass
``` shell
opt -load llvm-src/build/lib/LLVMAllocSite.so -alloc-site XXX.ll -dump dir_store_results
```
