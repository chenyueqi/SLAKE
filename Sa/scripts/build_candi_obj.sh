#!/bin/sh
~/build_llvm/build/bin/opt -load ~/build_llvm/build/lib/LLVMParseDS.so \
	-pds $1 \
	-dump $2 \
	-deref-step $3
