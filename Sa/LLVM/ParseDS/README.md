# What this pass do
extract all structures can possible dereference a function ptr in several steps

# How to add this pass to LLVM
```shell
cp ../ParseDs llvm-src/lib/Transforms
```
add
```
add_subdirectory(ParseDS)
```
to llvm-src/lib/Transforms/CMakeLists.txt


# How to build this pass
```shell
cd llvm-src/build
make
```

# How to use this pass
``` shell
opt -load llvm-src/build/lib/LLVMParseDS.so -pds XXX.ll -dump dir_to_store_results -deref-step maxDerefStep
```
