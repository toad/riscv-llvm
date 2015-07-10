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
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/GlobalValue.h"
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
      AttributeSet fnAttributes;
      Function *f = M.getFunction("__llvm_riscv_check_tagged");
      if(f) {
        FunctionCheckTagged = f;
        return false;
      }
      errs() << "Trying to create __llvm_riscv_check_tagged...\n";
      Type *int_type = IntegerType::get(Context, 8);
      errs() << "Trying to create pointer type...\n";
      Type *params = { PointerType::getUnqual(int_type) };
      errs() << "Got type for parameters\n";
      FunctionType *type = FunctionType::get(Type::getVoidTy(Context), params,
                                             false);
      errs() << "Got type for function\n";
      f = Function::Create(type, GlobalValue::LinkOnceODRLinkage,
                           "__llvm_riscv_check_tagged", &M);
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
      // FIXME add argument attribute NonNull, Dereferenceable to pointer.
      f -> addFnAttr(Attribute::AlwaysInline);
      f -> addFnAttr(Attribute::NoCapture);
      f -> addFnAttr(Attribute::ReadOnly);

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
      bool doneSomething = false;
      errs() << "TagCodePointers running on basic block...\n";
      BasicBlock::InstListType& instructions = BB.getInstList();
      for(BasicBlock::InstListType::iterator it = instructions.begin(); 
          it != BB.end(); it++) {
        Instruction& inst = *it;
        //errs() << "Instruction " << inst << "\n";
        if(StoreInst::classof(&inst)) {
          StoreInst& s = (StoreInst&) inst;
          Value *ptr = s.getPointerOperand();
          Type *type = ptr -> getType();
          errs() << "Detected a store, type: ";
          assert(isa<PointerType>(type) &&
            "parameter must be a pointer.");
          PointerType *t = (PointerType*) type;
          t -> print(errs());
          errs() << "\n";
          bool shouldTag = shouldTagType(t);
          if(!shouldTag && isInt8Pointer(t) && isa<Instruction>(ptr)) {
            errs() << "Hmmm....\n";
            shouldTag = shouldTagBitCastInstruction(ptr);
          }
          if(shouldTag) {
            errs() << "Should tag the store!\n";
            createSTag(instructions, it, ptr, BB.getParent()->getParent());
            doneSomething = true;
          }
        } else if(LoadInst::classof(&inst)) {
          LoadInst& l = (LoadInst&) inst;
          Value *ptr = l.getPointerOperand();
          Type *type = ptr -> getType();
          errs() << "Detected a load, type: ";
          assert(isa<PointerType>(type) &&
            "parameter must be a pointer.");
          PointerType *t = (PointerType*) type;
          t -> print(errs());
          errs() << "\n";
          bool shouldTag = shouldTagType(t);
          if(!shouldTag && isInt8Pointer(t) && isa<Instruction>(ptr)) {
            errs() << "Hmmm....\n";
            assert(isa<Instruction>(ptr));
            shouldTag = shouldTagBitCastInstruction(ptr);
          }
          if(shouldTag) {
            errs() << "Should tag the store!\n";
            createCheckTagged(instructions, it, ptr, BB.getParent()->getParent());
            doneSomething = true;
          }
        }
      }
      getFunctionCheckTagged();
      return doneSomething;
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

    /** Common idiom in generated IR: Bitcast before store. */
    bool shouldTagBitCastInstruction(Value *ptr) {
      // FIXME could this be outside the current basic block??
      // FIXME can we tell in advance or do we need a FunctionPass after all???
      errs() << "Previous instruction: " << *ptr << "\n";
      if(isa<BitCastInst>(ptr)) {
        // Common idiom in generated code with TBAA turned off: Bitcast to i8*[*...].
        // E.g. when setting vptr's.
        Type *type = ((BitCastInst*)ptr)->getSrcTy();
        errs() << "Type is really: ";
        type -> print(errs());
        errs() << "\n";
        return shouldTagType(type);
      } else return false;
    }
    
    // FIXME lots of duplication getting function-level-or-higher globals here
    // FIXME duplication with IRBuilder.
    
    /* Insert stag(ptr, TAG_CLEAN) after the current instruction. */
    void createSTag(BasicBlock::InstListType& instructions, 
                         BasicBlock::InstListType::iterator it, Value *Ptr, Module *M) {
      assert(isa<PointerType>(Ptr->getType()) &&
       "stag only applies to pointers.");
      LLVMContext &Context = M->getContext();
      Ptr = getCastedInt8PtrValue(Context, instructions, it, Ptr); // FIXME consider int64 ptr
      Value *TagValue = getInt64(llvm::IRBuilderBase::TAG_CLEAN, Context);
      Value *Ops[] = { TagValue, Ptr };
      Value *TheFn = Intrinsic::getDeclaration(M, Intrinsic::riscv_stag);
      CallInst *CI = CallInst::Create(TheFn, Ops, "");
      // Set tag afterwards.
      instructions.insertAfter(it, CI);
    }

    /* Call __llvm_riscv_check_tagged before the current instruction */
    void createCheckTagged(BasicBlock::InstListType& instructions, 
                         BasicBlock::InstListType::iterator it, Value *Ptr, Module *M) {
      assert(isa<PointerType>(Ptr->getType()) &&
       "stag only applies to pointers.");
      LLVMContext &Context = M->getContext();
      Ptr = getCastedInt8PtrValue(Context, instructions, it, Ptr); // FIXME consider int64 ptr
      Value *Ops[] = { Ptr };
      CallInst *CI = CallInst::Create(getFunctionCheckTagged(), Ops, "");
      instructions.insert(it, CI);
    }
    
    /// \brief Get a constant 64-bit value.
    ConstantInt *getInt64(uint64_t C, LLVMContext &Context) {
      return ConstantInt::get(Type::getInt64Ty(Context), C);
    }

    Value *getCastedInt8PtrValue(LLVMContext &Context,
                         BasicBlock::InstListType& instructions, 
                         BasicBlock::InstListType::iterator it,
                         Value *Ptr) {
      PointerType *PT = cast<PointerType>(Ptr->getType());
      if (PT->getElementType()->isIntegerTy(8))
        return Ptr;
      
      // Otherwise, we need to insert a bitcast.
      PT = Type::getInt8PtrTy(Context, PT->getAddressSpace());
      BitCastInst *BCI = new BitCastInst(Ptr, PT, "");
      instructions.insert(it, BCI);
      return BCI;
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

namespace llvm {
  BasicBlockPass* createLowRISCTagCodePointersPass() {
    return new TagCodePointers();
  }
}

