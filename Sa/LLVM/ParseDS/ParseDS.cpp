//===- ParseDS.cpp -- ParseDataStructure------------------------------//

/*
 * @file: PDSPass.cpp
 * @author: Yueqi Chen
 * @date: 03/09/2018
 * @version: 1.0
 *
 * @section LICENSE
 *
 * @section DESCRIPTION
 *
 */

#include "ParseDS.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

char ParseDSPass::ID = 0;
static RegisterPass<ParseDSPass> PARSEDS("pds", "Parse Data Structure");
static cl::opt<std::string> corpusDir("dump", cl::desc("path to store corpus"), 
										cl::value_desc("filename"));
static cl::opt<unsigned int> maxDerefStep("deref-step", cl::desc("max dereference step to func"),
										cl::value_desc("unsigned int"));

ParseDSPass::~ParseDSPass() {}

bool ParseDSPass::runOnModule(llvm::Module& M) {
	dataLayout =  new DataLayout(&M);
	std::vector<StructType*> structTypeVec = M.getIdentifiedStructTypes();
	for (StructType* &structType : structTypeVec) {
		if (!structType->hasName())
			continue;
		if (!isTargetObj(structType, 0))
			continue;
		dumpCorpus(structType);
	}
	return false;
}

bool ParseDSPass::isTargetObj(StructType* structType, unsigned int currentDerefStep) {
	if (currentDerefStep >= maxDerefStep)
		return false;
	unsigned int i = 0;
	for (StructType::element_iterator iter = structType->element_begin();
			iter != structType->element_end(); iter++) {
		Type* subfieldType = structType->getElementType(i++);
		if (subfieldType->getTypeID() == Type::PointerTyID) { // pointer subfield
			Type* pointeeType =((PointerType*)subfieldType)->getElementType();
			if(pointeeType->getTypeID() == Type::FunctionTyID)
				return true;
			else if (pointeeType->getTypeID() == Type::StructTyID)
				if (isTargetObj((StructType*)pointeeType, currentDerefStep+1))
					return true;
		} else if (subfieldType->getTypeID() == Type::StructTyID) { // nested structure
			if (isTargetObj((StructType*)subfieldType, currentDerefStep+1))
				return true;
		}
	}
	return false;
}

/*
void ParseDSPass::dumpFormat(FILE* fp, const char* name, unsigned int currentDerefStep) {
	for(unsigned int i = 0 ; i < currentDerefStep; i++)
		fprintf(fp, "<-");
	fprintf(fp, "%s\n", name);
}
*/

void ParseDSPass::dumpFormat(FILE* fp, const char* name, unsigned int offset, unsigned int currentDerefStep) {
	for (unsigned int i = 0; i < currentDerefStep; i++)
		fprintf(fp, "<");
	fprintf(fp, "%s:%u\n", name, offset);
}

bool ParseDSPass::dumpStruct(FILE* fp, StructType* structType, unsigned int currentDerefStep) {
	bool ret = false;

	if (currentDerefStep >= maxDerefStep)
		return ret;

	if (structType->isOpaque())
		return ret;

	const StructLayout* SL = dataLayout->getStructLayout(structType);

	unsigned int i = 0;
	for (StructType::element_iterator iter = structType->element_begin(); 
			iter != structType->element_end(); iter++, i++) {
		Type* subfieldType = structType->getElementType(i);
		if (subfieldType->getTypeID() == Type::PointerTyID) { // pointer subfield
			Type* pointeeType =((PointerType*)subfieldType)->getElementType();
			if(pointeeType->getTypeID() == Type::FunctionTyID) {
				// dumpFormat(fp, "func", SL->getElementOffset(i), currentDerefStep);
				ret = true;
			} else if (pointeeType->getTypeID() == Type::StructTyID) {
				if (((StructType*)pointeeType)->hasName() && 
						dumpStruct(fp, (StructType*)pointeeType, currentDerefStep+1)) {
					dumpFormat(fp, ((StructType*)pointeeType)->getName().data(), 
								SL->getElementOffset(i), currentDerefStep);
					ret = true;
				}
			}
		} else if (subfieldType->getTypeID() == Type::StructTyID) { // nested structure
			if (((StructType*)subfieldType)->hasName() && 
					dumpStruct(fp, (StructType*)subfieldType, currentDerefStep+1)) {
				dumpFormat(fp, ((StructType*)subfieldType)->getName().data(), 
							SL->getElementOffset(i), currentDerefStep);
				ret = true;
			}
		}
	}
	return ret;
}

void ParseDSPass::dumpCorpus(StructType* structType) {
	char corpus_file[512];
	memset(corpus_file, '\0', 512);
	strcat(corpus_file, corpusDir.c_str());
	strcat(corpus_file, structType->getName().data());
	if (access(corpus_file, F_OK) != -1) return;
	FILE* fp = fopen(corpus_file, "w");
	// fprintf(fp, "struct name:\t%s\n", structType->getName().data());
	// fprintf(fp, "struct size:\t%d\n", getSize(structType));
	dumpStruct(fp, structType, 0);
	fclose(fp);
}

unsigned int ParseDSPass::getSize(Type* type) {
	switch(type->getTypeID()) {
		case Type::IntegerTyID:
			return getSize((IntegerType*)type);
		case Type::StructTyID:
			return getSize((StructType*)type);
		case Type::PointerTyID:
			return getSize((PointerType*)type);
		case Type::ArrayTyID:
			return getSize((ArrayType*)type);
		default:
			fprintf(stderr, "TypeID %d hasn't been implemented\n", type->getTypeID());
			break;
	}
	return 0;
}

unsigned int ParseDSPass::getSize(IntegerType* type) {
	return type->getBitWidth()>>3;
}

unsigned int ParseDSPass::getSize(StructType* type) {
	unsigned int i = 0;
	unsigned int size_sum = 0;
	for (StructType::element_iterator iter = type->element_begin();
			iter != type->element_end(); iter++) {
		Type* t = type->getElementType(i);
		size_sum += getSize(t);
		i++;
	}
	return size_sum;
}

unsigned int ParseDSPass::getSize(PointerType* type) {
	return 8; // 64 bit 
}

unsigned int ParseDSPass::getSize(ArrayType* type) {
	Type* element_type = type->getArrayElementType();
	uint64_t element_num = type->getArrayNumElements();
	// TODO we can check if struct size is mutable here
	// by heuristic rules
	return element_num*getSize(element_type);
}
