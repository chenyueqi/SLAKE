//===- AllocSite.cpp -- record functions allocating target object------//

/*
 * @file: AllocSite.cpp
 * @author: Yueqi Chen
 * @date: 01/02/2019
 * @version: 1.0
 *
 * @section LICENSE
 *
 * @section DESCRIPTION
 *
 */

#include "AllocSite.h"
#include "llvm/Support/CommandLine.h"
using namespace llvm;

char AllocSitePass::ID = 0;
static RegisterPass<AllocSitePass> ALLOCSITE("alloc-site",
        "Records functions allocating target object");
static cl::opt<std::string> corpusDir("dump", cl::desc("path to store corpus"), 
																				cl::value_desc("filename"));

AllocSitePass::~AllocSitePass() {}

bool AllocSitePass::runOnFunction(Function& F) {
	for (BasicBlock &BB : F) 
		for (Instruction &I : BB)
			if (CallInst *inst = dyn_cast<CallInst>(&I)) {
				const Function* Callee = inst->getCalledFunction();

				// Callee can be NULL if direct calls are in the generated assembly
				// For some reason, Function::getCalledFunction() 
				// does not strip off pointer casts on its first arguments. 
				// Therefore, we need to do this. 
				if (!Callee)
					Callee = dyn_cast<Function>(inst->getCalledValue()->stripPointerCasts());
				if (Callee) {
					std::string calleeName = Callee->getName().str();				
					if (isAllocFunction(calleeName))
						recordRetType(F, *inst);
				}
			}
	return false;
}

bool AllocSitePass::isAllocFunction(std::string calleeName) {
	if (
			// general
			calleeName == "__kmalloc" || 
			calleeName == "__kmalloc_node" ||
			calleeName == "kmalloc" ||
			calleeName == "kmalloc_node" ||
			calleeName == "kmalloc_array" ||
			calleeName == "kzalloc" ||
			calleeName == "kmalloc_array_node" ||
			calleeName == "kzalloc_node" ||
			calleeName == "kcalloc_node" ||
			calleeName == "kcalloc" ||
			// special
			calleeName == "kmem_cache_alloc" ||
			calleeName == "kmem_cache_alloc_node" ||
			calleeName == "kmem_cache_zalloc" ||
			// corner
			calleeName == "sock_kmalloc"
			) 
		return true;
	else
		return false;
}

void AllocSitePass::recordRetType(Function& F, CallInst& I) {
	// The CallInst is its return value. They are one and the same.
	for (Value::use_iterator ui = I.use_begin(); ui != I.use_end(); ui++) {
		if (CastInst* inst = dyn_cast<CastInst>(ui->getUser())) {
			Type* T = inst->getDestTy();
			if (T->getTypeID() == Type::PointerTyID) {
				Type* elementType = ((PointerType*)T)->getElementType();
				if (elementType->getTypeID() == Type::StructTyID) {
					const char* elementTypeName = elementType->getStructName().data();
					dumpCorpus(F, elementTypeName);
				}
			}
		}
	} 
}

void AllocSitePass::dumpCorpus(Function& F, const char* elementTypeName) {
	char corpus_file[512];
	memset(corpus_file, '\0', 512);
	strcat(corpus_file, corpusDir.c_str());
	strcat(corpus_file, elementTypeName);
	fprintf(stdout, "searching file:\t%s\n", corpus_file);
	FILE* fp = fopen(corpus_file, "a+");
	fprintf(fp, "%s\n", F.getName().data());
	fclose(fp);
}
