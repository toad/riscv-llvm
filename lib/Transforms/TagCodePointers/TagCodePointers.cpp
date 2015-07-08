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
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
  struct TagCodePointers : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    TagCodePointers() : FunctionPass(ID) {}

    Function* FunctionCheckTagged = NULL;

    /* Adds __llvm_riscv_check_tagged */
    virtual bool doInitialization(Module &M) {
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
      return true;
    }

    virtual bool runOnFunction(Function &F) {
      errs() << "TagCodePointers running on function ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }
  };
}

char TagCodePointers::ID = 0;
static RegisterPass<TagCodePointers> X("tag-code-pointers", "Tag code pointers (LowRISC)");


