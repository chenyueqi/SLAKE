//===- ParseDS.h -- Parse Data Structure------------------------------------//

/*
 * @file: ParseDS.h
 * @author: Yueqi Chen
 * @date: 03/10/2017
 * @version: 1.0
 *
 * @section LICENSE
 *
 * @section DESCRIPTION
 *
 */

#ifndef ParseDS_H_
#define ParseDS_H_

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DataLayout.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <cstring>

class ParseDSPass: public llvm::ModulePass { 
public:
    static char ID;
	llvm::DataLayout* dataLayout;
    ParseDSPass() : llvm::ModulePass(ID) {}
    ~ParseDSPass();

    virtual bool runOnModule(llvm::Module& module);
    virtual inline llvm::StringRef getPassName() const {
        return "ParseDSPass";
    }

private:
		unsigned int getSize(llvm::Type* type);
		unsigned int getSize(llvm::IntegerType* type);
		unsigned int getSize(llvm::StructType* type);
		unsigned int getSize(llvm::PointerType* type);
		unsigned int getSize(llvm::ArrayType* type);

		bool isTargetObj(llvm::StructType* type, unsigned int currentDerefStep);
		void dumpCorpus(llvm::StructType* type);
		void dumpFormat(FILE* fp, const char* name, unsigned int offset, unsigned int currentDerefStep);
		bool dumpStruct(FILE* fp, llvm::StructType* structType, unsigned int currentDerefStep);
};

#endif
