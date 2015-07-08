//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// LowRISC control flow hijacking protection.
// Sets CLEAN when writing a code pointer.
// Checks for CLEAN when reading a code pointer (or aborts).
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "tagcodepointers"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
  struct TagCodePointersBase : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    TagCodePointersBase() : ModulePass(ID) {}

    Function* FunctionCheckTagged = NULL;

    /* Adds __llvm_riscv_check_tagged */
    virtual bool runOnModule(Module &M) {
      LLVMContext &Context = M.getContext();
      // FIXME reconsider linkage - want people to be able to override it?
      Constant* c = M.getOrInsertFunction("__llvm_riscv_check_tagged",
                                         Type::getVoidTy(Context), // return type
                                         PointerType::getUnqual(IntegerType::get(Context, 8)),
                                         NULL);
      Function *f = cast<Function> (c);
      Function::arg_iterator args = f->arg_begin();
      Value *ptr = args++;
      BasicBlock* entry = BasicBlock::Create(Context, "entry", f);
      BasicBlock* onTagged = BasicBlock::Create(Context, "entry", f);
      BasicBlock* onNotTagged = BasicBlock::Create(Context, "entry", f);
      IRBuilder<> builder(entry);
      Value *ltag = builder.CreateRISCVLoadTag(ptr);
      Value *ltagEqualsOne = builder.CreateICmpEQ(ltag, builder.getInt64(IRBuilderBase::TAG_CLEAN));
      builder.CreateCondBr(ltagEqualsOne, onTagged, onNotTagged);
      builder.SetInsertPoint(onTagged);
      builder.CreateRetVoid();
      builder.SetInsertPoint(onNotTagged);
      builder.CreateTrap();
      builder.CreateRetVoid(); // FIXME Needs a terminal?
      FunctionCheckTagged = f;
      errs() << "TagCodePointersBase added function\n";
      return true;
    }
    
    Function* getFunctionCheckTagged() {
      return FunctionCheckTagged;
    }

  };

  char TagCodePointersBase::ID = 0;
  static RegisterPass<TagCodePointersBase> X("tag-code-pointers-base", "Add helper function for tag-code-pointers");

  struct TagCodePointers : public BasicBlockPass {
    
    Function *FunctionCheckTagged = NULL;

    static char ID; // Pass identification, replacement for typeid
    TagCodePointers() : BasicBlockPass(ID) {}

    virtual void getAnalysisUsage(AnalysisUsage &Info) const {
      Info.addRequired<TagCodePointersBase>();
    }

    virtual bool runOnBasicBlock(BasicBlock &BB) {
      errs() << "TagCodePointers running on basic block...\n";
      for(BasicBlock::InstListType::iterator it = BB.getInstList().begin(); 
          it != BB.end(); it++) {
        Instruction& inst = *it;
        //errs() << "Instruction " << inst << "\n";
        if(StoreInst::classof(&inst)) {
          StoreInst& s = (StoreInst&) inst;
          Value *ptr = s.getPointerOperand();
          Type *type = ptr -> getType();
          errs() << "Detected a store type: ";
          assert(isa<PointerType>(type) &&
            "parameter must be a pointer.");
          PointerType *t = (PointerType*) type;
          t -> print(errs());
          errs() << "\n";
          bool shouldTag = shouldTagType(t);
          if(!shouldTag && isInt8Pointer(t)) {
            errs() << "Hmmm....\n";
            assert(isa<Instruction>(ptr));
            // FIXME could this be outside the current basic block??
            // FIXME can we tell in advance or do we need a FunctionPass after all???
            errs() << "Previous instruction: " << *ptr << "\n";
            if(isa<BitCastInst>(ptr)) {
              // Common idiom in generated code with TBAA turned off: Bitcast to i8*[*...].
              // E.g. when setting vptr's.
              type = ((BitCastInst*)ptr)->getSrcTy();
              errs() << "Type is really: ";
              type -> print(errs());
              errs() << "\n";
              shouldTag = shouldTagType(type);
            }
          }
          if(shouldTag) {
            errs() << "Should tag the store!\n";
            // FIXME insert stag
          }
        }
      }
      getFunctionCheckTagged();
      return false;
    }
    
    /* Returns true if the type includes or refers to a function pointer */
    bool shouldTagType(Type *type) {
      if(isa<FunctionType>(type)) {
        return true;
      } else if(isa<SequentialType>(type)) {
        return shouldTagType(((SequentialType*)type) -> getElementType());
      } else if(isa<StructType>(type)) {
        StructType *s = (StructType*) type;
        for(StructType::element_iterator it = s -> element_begin();
            it != s -> element_end();) {
          if(shouldTagType(*it)) return true;
        }
        return false;
      } else {
        return false;
      }
    }
    
    /* LLVM often bitcasts wierd pointers to i8*** etc before storing... */
    bool isInt8Pointer(Type *type) {
      if(isa<PointerType>(type)) {
        return isInt8Pointer(((PointerType*)type) -> getElementType());
      } else if(isa<IntegerType>(type)) {
        return ((IntegerType*)type)->getBitWidth() == 8;
      } else return false;
    }
    
    Function *getFunctionCheckTagged() {
      if(FunctionCheckTagged) return FunctionCheckTagged;
      FunctionCheckTagged = getAnalysis<TagCodePointersBase>().getFunctionCheckTagged();
      errs() << "TagCodePointers got " << FunctionCheckTagged << "\n";
      return FunctionCheckTagged;
    }
  };
  
  char TagCodePointers::ID = 0;
  static RegisterPass<TagCodePointers> Y("tag-code-pointers", "Tag code pointers (LowRISC control flow protection)");
}

