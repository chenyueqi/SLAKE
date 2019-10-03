//===- AllocSite.h -- record functions allocating target object-------//

/*
 * @file: AllocSite.h
 * @author: Yueqi Chen
 * @date: 01/02/2019
 * @version: 1.0
 *
 * @section LICENSE
 *
 * @section DESCRIPTION
 *
 */

#ifndef AllocSite_H_
#define AllocSite_H_

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <cstring>

class AllocSitePass: public llvm::FunctionPass { 
public:
    static char ID; // Pass ID
    AllocSitePass() : llvm::FunctionPass(ID) {}
    ~AllocSitePass();

    /// We start from here
    virtual bool runOnFunction(llvm::Function& Func);
    virtual inline llvm::StringRef getPassName() const {
        return "AllocSitePass";
    }
private:
		bool isAllocFunction(std::string calleeName);
		void recordRetType(llvm::Function& Func, llvm::CallInst& Inst);
		void dumpCorpus(llvm::Function& Func, const char* ElementTypeName);
};
#endif
