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
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {

  /// \brief Get a constant 64-bit value.
  ConstantInt *getInt64(uint64_t C, LLVMContext &Context) {
    return ConstantInt::get(Type::getInt64Ty(Context), C);
  }

  struct TagCodePointersBase : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    TagCodePointersBase() : ModulePass(ID) {}

    Function* FunctionCheckTagged = NULL;
    Function* Init = NULL;

    /* Adds __llvm_riscv_check_tagged */
    virtual bool runOnModule(Module &M) {
      LLVMContext &Context = M.getContext();
      return addCheckTagged(M, Context) | tagGlobals(M, Context);
    }

    bool tagGlobals(Module &M, LLVMContext &Context) {
      if(!shouldTagGlobals(M)) return false;

      AttributeSet fnAttributes;
      FunctionType *type = FunctionType::get(Type::getVoidTy(Context), false);
      Function *init = Function::Create(type, GlobalValue::PrivateLinkage,
                           "__llvm_riscv_init", &M);
      BasicBlock* entry = BasicBlock::Create(Context, "entry", init);
      IRBuilder<> builder(entry);

      Value *TagValue = getInt64(llvm::IRBuilderBase::TAG_CLEAN, Context);

      errs() << "Function at start: \n" << *init << "\n";

      for(Module::GlobalListType::iterator it = M.getGlobalList().begin();
        it != M.getGlobalList().end(); it++) {
        GlobalVariable& var = *it;
        if(var.hasInitializer()) {
          errs() << "Initialized variable: " << var << "\n";
          errs() << "Type is " << *var.getType() << "\n";
          Constant *initializer = var.getInitializer();
          if(!checkInitializer(initializer)) continue;
          // FIXME probably need to pass builder in?
          std::list<Value*> toTag = processInitializer(initializer, &var, Context, builder);
          for(std::list<Value*>::iterator it = toTag.begin(); it != toTag.end(); it++) {
            errs() << "Must tag: \n" << **it << "\n\n";
            builder.CreateRISCVStoreTag(*it, TagValue);
          }
        } else {
          errs() << "Declared variable: " << var << "\n";
        }
      }

      builder.CreateRetVoid();

      errs() << "Function at end: \n" << *init << "\n";

      // FIXME use global_ctors.
      // FIXME doesn't work at the moment because of having to link with gcc
      // Easily demonstrated by trying to use iostreams. :(

      Init = init;
      return true;
    }

    bool shouldTagGlobals(Module &M) {
      for(Module::GlobalListType::iterator it = M.getGlobalList().begin();
        it != M.getGlobalList().end(); it++) {
        GlobalVariable& var = *it;
        if(var.hasInitializer()) {
          errs() << "Initialized variable: " << var << "\n";
          errs() << "Type is " << *var.getType() << "\n";
          Constant *initializer = var.getInitializer();
          if(checkInitializer(initializer)) {
            errs() << "Something to tag...\n";
            return true;
          }
        } else {
          errs() << "Declared variable: " << var << "\n";
        }
      }
      return false;
    }

    bool checkInitializer(Constant *init) {
      std::list<Value*> grabbers;
      if(init->isZeroValue()) {
        // Ignore.
      } else if(isa<BlockAddress>(init)) {
        return true;
      } else if(isa<ConstantInt>(init)) {
      } else if(isa<ConstantFP>(init)) {
      } else if(isa<ConstantAggregateZero>(init)) {
      } else if(isa<ConstantArray>(init)) {
        return checkOperands(init);
      } else if(isa<ConstantStruct>(init)) {
        return checkOperands(init);
      } else if(isa<ConstantVector>(init)) {
        return checkOperands(init);
      } else if(isa<ConstantDataSequential>(init)) {
      } else if(isa<ConstantExpr>(init)) {
        ConstantExpr *expr = (ConstantExpr*) init;
        if(expr->isCast()) {
          Value *op = (Constant*) (expr->getOperand(0));
          return checkInitializer((Constant*)op);
        } // Else assume it's harmless...
      } else if(isa<UndefValue>(init)) {
      } else if(isa<GlobalVariable>(init)) {
        // Ignore.
      } else if(isa<Function>(init)) {
        return true;
        // FIXME GlobalAlias
        // FIXME GlobalObject
      } else {
        errs() << "**** Constant is unrecognised!\n";
        errs() << *init << "\n\n";
      }
      return false;
    }

    std::list<Value*> processInitializer(Constant *init, Value *getter, LLVMContext &Context, IRBuilder<> &builder) {
      std::list<Value*> grabbers;
      if(!checkInitializer(init)) return grabbers;
      errs() << "Processing:\n" << *init << "\n\n";
      if(init->isZeroValue()) {
        errs() << "Constant is zero/null, ignoring...\n";
      } else if(isa<BlockAddress>(init)) {
        errs() << "**** MUST TAG: Constant is a block address!\n";
        grabbers.push_front(getter);
        return grabbers;
      } else if(isa<ConstantInt>(init)) {
        errs() << "Constant integer, ignoring...\n";
      } else if(isa<ConstantFP>(init)) {
        errs() << "Constant float, ignoring...\n";
      } else if(isa<ConstantAggregateZero>(init)) {
        errs() << "Constant aggregate zero, ignoring...\n";
      } else if(isa<ConstantArray>(init)) {
        errs() << "Constant is an array, must descend...\n";
        return processOperands(init, getter, Context, builder);
      } else if(isa<ConstantStruct>(init)) {
        errs() << "Constant is a struct, must descend...\n";
        return processOperands(init, getter, Context, builder);
      } else if(isa<ConstantVector>(init)) {
        errs() << "Constant is a vector, must descend...\n";
        return processOperands(init, getter, Context, builder);
      } else if(isa<ConstantDataSequential>(init)) {
        errs() << "Constant is a constant data sequence...\n";
      } else if(isa<ConstantExpr>(init)) {
        errs() << "Constant is a constant expression...\n";
        ConstantExpr *expr = (ConstantExpr*) init;
        errs() << "Constant opcode is " << expr->getOpcodeName() << "\n";
        if(expr->isCast()) {
          errs() << "Constant is a cast...\n";
          Value *op = (Constant*) (expr->getOperand(0));
          // Cast is irrelevant, we will need to cast at the end anyway.
          return processInitializer((Constant*)op, getter, Context, builder);
        } // Else assume it's harmless...
      } else if(isa<UndefValue>(init)) {
        errs() << "Constant is undefined...\n";
      } else if(isa<GlobalVariable>(init)) {
        errs() << "Constant is another global variable...\n";
        // Ignore.
      } else if(isa<Function>(init)) {
        errs() << "**** MUST TAG: Constant is a function!\n";
        grabbers.push_front(getter);
        return grabbers;
        // FIXME GlobalAlias
        // FIXME GlobalObject
      } else {
        errs() << "**** Constant is unrecognised!\n";
        errs() << *init << "\n\n";
      }
      return grabbers;
    }

    std::list<Value*> processOperands(Constant *init, Value *getter, LLVMContext &Context, IRBuilder<> &builder) {
      std::list<Value*> grabbers;
      for(unsigned i=0;i<init->getNumOperands();i++) {
        errs() << "Processing aggregate parameter " << i << "\n";
        Value *v = init->getOperand(i);
        if(!checkInitializer((Constant*)v)) continue;
        errs() << "Parameter is \n" << *v << "\n";
        if(!isa<Constant>(*v)) {
          errs() << "Member is not a constant?!\n";
        } else {
          std::vector<Value*> args;
          args.push_back(getInt64(0, Context)); // Dereference pointer to structure.
          args.push_back(getInt64(i, Context)); // Choose element.
          std::list<Value*> sub = processMoreOperands((Constant*)v, getter, args, Context, builder);
          grabbers.splice(grabbers.begin(), sub);
        }
      }
      return grabbers;
    }
    
    std::list<Value*> processMoreOperands(Constant *init, Value *getter, std::vector<Value*> derefSoFar, LLVMContext &Context, IRBuilder<> builder) {
      std::list<Value*> grabbers;
      if(isa<ConstantArray>(init) || isa<ConstantVector>(init) || isa<ConstantStruct>(init)) {
        std::vector<Value*> deref = derefSoFar;
        // Dereference more levels of nested (but packed!) array/structure...
        for(unsigned i=0;i<init->getNumOperands();i++) {
          errs() << "Processing aggregate parameter " << i << "\n";
          Value *v = init->getOperand(i);
          Constant *c = (Constant*)v;
          if(!checkInitializer(c)) continue;
          errs() << "Parameter is \n" << *v << "\n";
          deref.push_back(getInt64(i, Context));
          std::list<Value*> sub = 
            processMoreOperands(init, getter, deref, Context, builder);
          grabbers.splice(grabbers.begin(), sub);
        }
      } else {
        errs() << "Creating GetElementPtrInst from:\n";
        errs() << *getter;
        errs() << "\nDereference chain: " << "\n";
        for(std::vector<Value*>::iterator it = derefSoFar.begin(); 
            it != derefSoFar.end(); it++) {
          errs() << *it << "\n";
        }
        Value *newGetter = builder.CreateGEP(getter, derefSoFar);
        errs() << "Getter is " << *newGetter << "\n";
        std::list<Value*> sub = processInitializer(init, newGetter, Context, builder);
        grabbers.splice(grabbers.begin(), sub);
      }
      return grabbers;
    }

    bool checkOperands(Constant *init) {
      for(unsigned i=0;i<init->getNumOperands();i++) {
        Value *v = init->getOperand(i);
        if(!isa<Constant>(*v)) {
          errs() << "Member is not a constant?!\n";
        } else {
          if(checkInitializer((Constant*)v)) return true;
        }
      }
      return false;
    }

    bool addCheckTagged(Module &M, LLVMContext &Context) {
      // FIXME reconsider linkage - want people to be able to override it?
      AttributeSet fnAttributes;
      Function *f = M.getFunction("__llvm_riscv_check_tagged");
      if(f) {
        FunctionCheckTagged = f;
        return false;
      }
      Type *params = { PointerType::getUnqual(IntegerType::get(Context, 8)) };
      FunctionType *type = FunctionType::get(Type::getVoidTy(Context), params,
                                             false);
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

    Function* getFunctionInit() {
      return Init;
    }

  };

  char TagCodePointersBase::ID = 0;
  static RegisterPass<TagCodePointersBase> X("tag-code-pointers-base", "Add helper function for tag-code-pointers");

  struct TagCodePointers : public FunctionPass {
    
    Function *FunctionCheckTagged = NULL;
    Function *FunctionInit = NULL;

    static char ID; // Pass identification, replacement for typeid
    TagCodePointers() : FunctionPass(ID) {}

    virtual void getAnalysisUsage(AnalysisUsage &Info) const {
      Info.addRequired<TagCodePointersBase>();
    }

    virtual bool runOnFunction(Function &F) {
      StringRef name = F.getName();
      errs() << "Processing " << name << "\n";
      if(F.getName() == "__llvm_riscv_init" || 
         F.getName() == "__llvm_riscv_check_tagged") return false;
      bool added = false;
      Function::BasicBlockListType& blocks = F.getBasicBlockList();
      for(Function::BasicBlockListType::iterator it = blocks.begin(); 
          it != blocks.end(); it++) {
        if(runOnBasicBlock(*it)) added = true;
      }
      Function *Init = getFunctionInit();
      if(Init) {
        BasicBlock &entry = F.getEntryBlock();
        CallInst *CI = CallInst::Create(Init);
        entry.getInstList().insert(entry.getFirstInsertionPt(), CI);
      }
      return added;
    }

    bool runOnBasicBlock(BasicBlock &BB) {
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

    Function *getFunctionInit() {
      if(FunctionInit) return FunctionInit;
      FunctionInit = getAnalysis<TagCodePointersBase>().getFunctionInit();
      return FunctionInit;
    }

  };
  
  char TagCodePointers::ID = 0;
  static RegisterPass<TagCodePointers> Y("tag-code-pointers", "Tag code pointers (LowRISC control flow protection)");
}

