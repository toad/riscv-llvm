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
#include "llvm/Support/ConstantFolder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
using namespace llvm;

/* If true, tag pointers to structures including function pointers. */
static const bool TAG_SENSITIVE = true;
/* If true, tag void*. */
static const bool TAG_VOID = false;
/* If true, tag pointers to structures including void* pointers. */
static const bool TAG_SENSITIVE_VOID = false;
/* If true, tag all pointers */
static const bool TAG_POINTER = false;

namespace {

  /// \brief Get a constant 64-bit value.
  ConstantInt *getInt64(uint64_t C, LLVMContext &Context) {
    return ConstantInt::get(Type::getInt64Ty(Context), C);
  }

  Type* stripPointer(Type *type) {
    if(isa<PointerType>(type))
      return ((SequentialType*)type) -> getElementType();
    else return NULL;
  }

  /* Compute the appropriate tag to be written or expected for a pointer type. 
   * The outer pointer has been removed already. */
  IRBuilderBase::LowRISCMemoryTag shouldTagPointerType(Type *type) {
    if(isa<FunctionType>(type)) {
      return IRBuilderBase::TAG_CLEAN_FPTR;
    } else if(isa<SequentialType>(type)) {
      IRBuilderBase::LowRISCMemoryTag sub = 
        shouldTagPointerType(((SequentialType*)type) -> getElementType());
      switch(sub) {
        case IRBuilderBase::TAG_CLEAN_FPTR:
        case IRBuilderBase::TAG_CLEAN_PFPTR:
          return IRBuilderBase::TAG_CLEAN_PFPTR;
        case IRBuilderBase::TAG_CLEAN_SENSITIVE:
          return IRBuilderBase::TAG_CLEAN_SENSITIVE;
        case IRBuilderBase::TAG_CLEAN_SENSITIVE_VOID:
          return IRBuilderBase::TAG_CLEAN_SENSITIVE_VOID;
        case IRBuilderBase::TAG_CLEAN_VOIDPTR:
          if(TAG_SENSITIVE_VOID)
            return IRBuilderBase::TAG_CLEAN_SENSITIVE_VOID;
          // Fall through.
        default:
          if(TAG_POINTER) return IRBuilderBase::TAG_CLEAN_POINTER;
          return sub;
      }
    } else if((TAG_SENSITIVE || TAG_SENSITIVE_VOID) && isa<StructType>(type)) {
      StructType *s = (StructType*) type;
      bool foundVoid = false;
      for(StructType::element_iterator it = s -> element_begin();
          it != s -> element_end();it++) {
        IRBuilderBase::LowRISCMemoryTag sub = shouldTagPointerType(*it);
        if(TAG_SENSITIVE && (sub == IRBuilderBase::TAG_CLEAN_SENSITIVE || 
           sub == IRBuilderBase::TAG_CLEAN_FPTR || 
           sub == IRBuilderBase::TAG_CLEAN_PFPTR))
          return IRBuilderBase::TAG_CLEAN_SENSITIVE;
        if(TAG_SENSITIVE_VOID && (sub == IRBuilderBase::TAG_CLEAN_VOIDPTR ||
           sub == IRBuilderBase::TAG_CLEAN_SENSITIVE_VOID)) foundVoid = true;
      }
      if(foundVoid) return IRBuilderBase::TAG_CLEAN_SENSITIVE_VOID;
      return IRBuilderBase::TAG_NORMAL;
    } else if(TAG_VOID && type->isVoidTy()) {
      return IRBuilderBase::TAG_CLEAN_VOIDPTR; // Not a pointer yet but it will be.
    } else {
      return IRBuilderBase::TAG_NORMAL;
    }
  }

  IRBuilderBase::LowRISCMemoryTag shouldTagTypeOfWord(Type *type);

  /* Compute the correct type for a bare structure. That is, we are writing 
   * a pointer-sized word directly to a structure, what do we get? A classic 
   * example is setting the vptr. This is NOT the same as writing a pointer to
   * a structure. */
  IRBuilderBase::LowRISCMemoryTag shouldTagStructType(StructType *type) {
    for(StructType::element_iterator it = type -> element_begin();
        it != type -> element_end();it++) {
      // What's the first element?
      Type *element = *it;
      if(isa<StructType>(element)) {
        // A struct consisting of more structs back-to-back.
        return shouldTagStructType((StructType*)element);
      } else if(isa<PointerType>(element)) {
        // First element is a pointer.
        return shouldTagPointerType(stripPointer(element));
      } else if(isa<ArrayType>(element)) {
        // Is an array. We are interested in the first element.
        return shouldTagTypeOfWord(((SequentialType*)type) -> getElementType());
      } else if(element->isVoidTy()) {
        // Placeholder?
        continue;
      } else {
        // Something else. Not of interest.
        return IRBuilderBase::TAG_NORMAL;
      }
    }
    return IRBuilderBase::TAG_NORMAL;
  }

  /* Compute the tag to be expected, if any, for the object of a store or load.
   * We have stripped the outer pointer and are left with the type of the 
   * object in memory that we are about to write to / read from. A particularly
   * important point here: Writing to a structure means writing to its first
   * element! Same with an array. So e.g. writing to a C++ object with a vptr
   * usually means writing to its vptr. */
  IRBuilderBase::LowRISCMemoryTag shouldTagTypeOfWord(Type *type) {
    errs() << "shouldTag (word read/write): " << *type << "\n";
    if(isa<PointerType>(type)) {
      errs() << "Is a pointer type\n";
      // Pointer type. May be sensitive.
      return shouldTagPointerType(stripPointer((PointerType*)type));
    } else if(isa<StructType>(type)) {
      errs() << "Is a structure type\n";
      // Structure. We probably want the first element.
      return shouldTagStructType((StructType*)type);
    } else if(isa<ArrayType>(type)) {
      errs() << "Is an array type\n";
      // Array. We want the first element.
      return shouldTagTypeOfWord(((SequentialType*)type) -> getElementType());
    } else {
      // Otherwise it's not of interest.
      return IRBuilderBase::TAG_NORMAL;
    }
  }

  /* For an initializer, we don't know the width, and we are only interested
   * if it's a pointer. */
  IRBuilderBase::LowRISCMemoryTag shouldTagTypeInitializer(Type *type) {
    Type *innerType = stripPointer(type);
    if(innerType) return shouldTagPointerType(innerType);
    return IRBuilderBase::TAG_NORMAL;
  }

  /* LLVM often bitcasts wierd pointers to i8*** etc before storing... */
  bool isInt8Pointer(Type *type) {
    if(isa<PointerType>(type)) {
      return isInt8Pointer(((PointerType*)type) -> getElementType());
    } else if(isa<IntegerType>(type)) {
      return ((IntegerType*)type)->getBitWidth() == 8;
    } else return false;
  }

  struct TagCodePointersBase : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    TagCodePointersBase() : ModulePass(ID) {}

    Function* FunctionCheckTagged = NULL;

    /* Adds __llvm_riscv_check_tagged */
    virtual bool runOnModule(Module &M) {
      LLVMContext &Context = M.getContext();
      addSetupCSRs(M, Context);
      addCheckTagged(M, Context);
      tagGlobals(M, Context);
      return true;
    }
    
    static const long LD_TAG_CSR_VALUE = (1 << IRBuilderBase::TAG_WRITE_ONLY) | (1 << IRBuilderBase::TAG_INVALID);
    static const long SD_TAG_CSR_VALUE = (1 << IRBuilderBase::TAG_READ_ONLY) | (1 << IRBuilderBase::TAG_INVALID);
    
    void addSetupCSRs(Module &M, LLVMContext &Context) {
      // FIXME This is yet another gross hack. :)
      // This will add one global init per module.
      // It is not clear who will eventually be responsible for setting the LDCT/SDCT CSRs:
      // Will they be settable from userspace?
      // The kernel could enforce a single global policy, or it could save and restore values for each process.
      // FIXME In any case they should be set once per process, not once per module!
      // Fixing this requires support in the linker or the frontend.
      AttributeSet fnAttributes;
      Function *f = M.getFunction("__llvm_riscv_init_tagged_memory_csrs");
      if(f) {
        return;
      }
      FunctionType *type = FunctionType::get(Type::getVoidTy(Context),
                                             false);
      f = Function::Create(type, GlobalValue::LinkOnceAnyLinkage, // Allow overriding!
                           "__llvm_riscv_init_tagged_memory_csrs", &M);
      BasicBlock* entry = BasicBlock::Create(Context, "entry", f);
      IRBuilder<> builder(entry);
      Value *TheFn = Intrinsic::getDeclaration(&M, Intrinsic::riscv_set_tm_trap_ld);
      builder.CreateCall(TheFn, getInt64(LD_TAG_CSR_VALUE, Context));
      TheFn = Intrinsic::getDeclaration(&M, Intrinsic::riscv_set_tm_trap_sd);
      builder.CreateCall(TheFn, getInt64(SD_TAG_CSR_VALUE, Context));
      builder.CreateRetVoid();
      appendToGlobalCtors(M, f, 0);
    }

    bool tagGlobals(Module &M, LLVMContext &Context) {
      if(!shouldTagGlobals(M)) return false;

      AttributeSet fnAttributes;
      FunctionType *type = FunctionType::get(Type::getVoidTy(Context), false);
      Function *init = Function::Create(type, GlobalValue::PrivateLinkage,
                           "__llvm_riscv_init", &M);
      BasicBlock* entry = BasicBlock::Create(Context, "entry", init);
      IRBuilder<> builder(entry);

      errs() << "Function at start: \n" << *init << "\n";

      for(Module::GlobalListType::iterator it = M.getGlobalList().begin();
        it != M.getGlobalList().end(); it++) {
        GlobalVariable& var = *it;
        if(var.hasInitializer()) {
          errs() << "Initialized variable: " << var << "\n";
          if(var.hasAppendingLinkage()) {
            errs() << "Ignoring because of appending linkage\n";
            // Mostly this will be global_ctors / global_dtors, we don't need to do anything to them, they will be run anyway by lower level code.
            // FIXME do we need to support this?
            // Would need special-purpose code, how do we find the array length?
            continue;
          }
          errs() << "Type is " << *var.getType() << "\n";
          Constant *initializer = var.getInitializer();
          errs() << "Initializer is " << *initializer << "\n";
          if(!checkInitializer(initializer)) continue;
          processInitializer(initializer, &var, Context, builder, false);
        } else {
          errs() << "Declared variable: " << var << "\n";
        }
      }

      builder.CreateRetVoid();

      errs() << "Function at end: \n" << *init << "\n";

      appendToGlobalCtors(M, init, 0); // FIXME priority? 65534? Want to run before static constructors which may call functions!
      return true;
    }

    bool shouldTagGlobals(Module &M) {
      for(Module::GlobalListType::iterator it = M.getGlobalList().begin();
        it != M.getGlobalList().end(); it++) {
        GlobalVariable& var = *it;
        if(var.hasInitializer()) {
          errs() << "Initialized variable: " << var << "\n";
          if(var.hasAppendingLinkage()) {
            errs() << "Ignoring because of appending linkage\n";
            // Mostly this will be global_ctors / global_dtors, we don't need to do anything to them, they will be run anyway by lower level code.
            // FIXME do we need to support this?
            // Would need special-purpose code, how do we find the array length?
            continue;
          }
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

    /* Do we need to tag anything created during the initializer?
     * This is separate from the question of whether we need to tag the global
     * variable itself. */
    bool checkInitializer(Constant *init) {
      errs() << "Checking initializer " << *init << "\n";
      errs() << "Type is " << *(init->getType()) << "\n";
      if(init->isZeroValue()) {
        errs() << "Zero initializer, will be initialized later.\n";
        // Ignore.
        return false;
      }
      if(shouldTagTypeInitializer(init->getType()) != IRBuilderBase::TAG_NORMAL) {
        return true;
      }
      if(isa<BlockAddress>(init)) {
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
        // Ignore. We will have tagged it already.
        errs() << "Is global variable\n";
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

    /* Find anything inside the initializer that needs tagging. 
     * Returns a list of pointers to tag. */
    void processInitializer(Constant *init, Value *getter, LLVMContext &Context, IRBuilder<> &builder, bool addedAlready) {
      if(!checkInitializer(init)) return;
      if(!addedAlready && 
        (shouldTagTypeInitializer(init->getType()) != IRBuilderBase::TAG_NORMAL)) {
        errs() << "Adding to initializer because sensitive type: " << getter;
        errs() << "Initializer is " << *init << "\n";
        errs() << "Type is " << init->getType() << "\n";
        builder.CreateRISCVStoreTag(getter,
              getInt64(shouldTagTypeInitializer(init->getType()), Context));
        addedAlready = true;
      }
      errs() << "Processing:\n" << *init << "\n\n";
      if(init->isZeroValue()) {
        errs() << "Constant is zero/null, ignoring...\n";
      } else if(isa<ConstantInt>(init)) {
        errs() << "Constant integer, ignoring...\n";
      } else if(isa<ConstantFP>(init)) {
        errs() << "Constant float, ignoring...\n";
      } else if(isa<ConstantAggregateZero>(init)) {
        errs() << "Constant aggregate zero, ignoring...\n";
      } else if(isa<ConstantArray>(init)) {
        errs() << "Constant is an array, must descend...\n";
        processOperands(init, getter, Context, builder);
      } else if(isa<ConstantStruct>(init)) {
        errs() << "Constant is a struct, must descend...\n";
        processOperands(init, getter, Context, builder);
      } else if(isa<ConstantVector>(init)) {
        errs() << "Constant is a vector, must descend...\n";
        processOperands(init, getter, Context, builder);
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
          processInitializer((Constant*)op, getter, Context, builder, addedAlready);
        } // Else assume it's harmless...
      } else if(isa<UndefValue>(init)) {
        errs() << "Constant is undefined...\n";
      } else if(isa<GlobalVariable>(init)) {
        errs() << "Constant is another global variable...\n";
        // Ignore.
      } else if(isa<Function>(init)) {
        errs() << "**** MUST TAG: Constant is a function!\n";
        assert(shouldTagTypeInitializer(init->getType()) != IRBuilderBase::TAG_NORMAL);
        // FIXME GlobalAlias
        // FIXME GlobalObject
      } else {
        errs() << "**** Constant is unrecognised!\n";
        errs() << *init << "\n\n";
      }
    }

    void processOperands(Constant *init, Value *getter, LLVMContext &Context, IRBuilder<> &builder) {
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
          processMoreOperands((Constant*)v, getter, args, Context, builder);
        }
      }
    }
    
    void processMoreOperands(Constant *init, Value *getter, std::vector<Value*> derefSoFar, LLVMContext &Context, IRBuilder<> builder) {
      if(isa<ConstantArray>(init) || isa<ConstantVector>(init) || isa<ConstantStruct>(init)) {
        std::vector<Value*> deref = derefSoFar;
        // Dereference more levels of nested (but packed!) array/structure...
        for(unsigned i=0;i<init->getNumOperands();i++) {
          errs() << "Processing aggregate parameter " << i << " with " << derefSoFar.size() << " parameters already processed.\n";
          Value *v = init->getOperand(i);
          Constant *c = (Constant*)v;
          if(!checkInitializer(c)) continue;
          errs() << "Parameter is (processing more operands):\n" << *v << "\n";
          deref.push_back(getInt64(i, Context));
          processMoreOperands(c, getter, deref, Context, builder);
        }
      } else {
        errs() << "Creating GetElementPtrInst from:\n";
        errs() << *getter;
        errs() << "\nDereference chain: " << "\n";
        for(std::vector<Value*>::iterator it = derefSoFar.begin(); 
            it != derefSoFar.end(); it++) {
          errs() << **it << "\n";
        }
        Value *newGetter = builder.CreateGEP(getter, derefSoFar);
        errs() << "Getter is " << *newGetter << "\n";
        processInitializer(init, newGetter, Context, builder, false);
      }
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
    
    Function* addCheckTaggedFailed(Module &M, LLVMContext &Context) {
      AttributeSet fnAttributes;
      Function *f = M.getFunction("__llvm_riscv_check_tagged_failure");
      if(f) {
        return f;
      }
      FunctionType *type = FunctionType::get(Type::getVoidTy(Context),
                                             false);
      f = Function::Create(type, GlobalValue::LinkOnceAnyLinkage, // Allow overriding!
                           "__llvm_riscv_check_tagged_failure", &M);
      BasicBlock* entry = BasicBlock::Create(Context, "entry", f);
      IRBuilder<> builder(entry);
      builder.CreateTrap();
      builder.CreateRetVoid(); // FIXME Needs a terminal?
      return f;
    }

    bool addCheckTagged(Module &M, LLVMContext &Context) {
      AttributeSet fnAttributes;
      Function *f = M.getFunction("__llvm_riscv_check_tagged");
      if(f) {
        FunctionCheckTagged = f;
        return false;
      }
      Function *fail = addCheckTaggedFailed(M, Context);
      Type *params[] = { PointerType::getUnqual(IntegerType::get(Context, 8)), 
                       IntegerType::get(Context, 64) };
      FunctionType *type = FunctionType::get(Type::getVoidTy(Context), params,
                                             false);
      f = Function::Create(type, GlobalValue::LinkOnceODRLinkage,
                           "__llvm_riscv_check_tagged", &M);
      Function::arg_iterator args = f->arg_begin();
      Value *ptr = args++;
      Value *tagval = args++;
      BasicBlock* entry = BasicBlock::Create(Context, "entry", f);
      BasicBlock* onTagged = BasicBlock::Create(Context, "entry", f);
      BasicBlock* onNotTagged = BasicBlock::Create(Context, "entry", f);
      IRBuilder<> builder(entry);
      Value *ltag = builder.CreateRISCVLoadTag(ptr);
      Value *ltagEqualsOne = builder.CreateICmpEQ(ltag, tagval);
      builder.CreateCondBr(ltagEqualsOne, onTagged, onNotTagged);
      builder.SetInsertPoint(onTagged);
      builder.CreateRetVoid();
      builder.SetInsertPoint(onNotTagged);

      builder.CreateCall(fail);
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

  struct TagCodePointers : public FunctionPass {
    
    Function *FunctionCheckTagged = NULL;

    ConstantFolder Folder; // FIXME Remove when use IRBuilder for more.

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
      return added;
    }

    bool runOnBasicBlock(BasicBlock &BB) {
      Module *M = BB.getParent()->getParent();
      bool doneSomething = false;
      errs() << "TagCodePointers running on basic block...\n";
      std::vector<Instruction*> toDelete;
      BasicBlock::InstListType& instructions = BB.getInstList();
      for(BasicBlock::InstListType::iterator it = instructions.begin(); 
          it != BB.end(); it++) {
        Instruction& inst = *it;
        //errs() << "Instruction " << inst << "\n";
        if(StoreInst::classof(&inst)) {
          StoreInst& s = (StoreInst&) inst;
          if(s.getAlignment() != 0 && s.getAlignment() % 8 != 0) continue;
          Value *ptr = s.getPointerOperand();
          Type *type = ptr -> getType();
          errs() << "Detected a store, type: ";
          type -> print(errs());
          errs() << "\nStripped: ";
          Type *t = stripPointer(type);
          assert(t && "parameter must be a pointer.");
          t -> print(errs());
          errs() << "\n";
          IRBuilderBase::LowRISCMemoryTag shouldTag = shouldTagTypeOfWord(t);
          if(shouldTag == IRBuilderBase::TAG_NORMAL && 
             isInt8Pointer(t) && isa<Instruction>(ptr)) {
            errs() << "Hmmm....\n";
            shouldTag = shouldTagBitCastInstruction(ptr);
          }
          if(shouldTag != IRBuilderBase::TAG_NORMAL) {
            errs() << "Should tag the store: " << shouldTag << "\n";
            createStoreAndSetTag(instructions, it, s.getValueOperand(), ptr, M, shouldTag);
            errs() << "Added store and set tag\n";
            toDelete.push_back(&s);
            doneSomething = true;
          }
        } else if(LoadInst::classof(&inst)) {
          LoadInst& l = (LoadInst&) inst;
          if(l.getAlignment() != 0 && l.getAlignment() % 8 != 0) continue;
          Value *ptr = l.getPointerOperand();
          Type *type = ptr -> getType();
          errs() << "Detected a load, type: ";
          type -> print(errs());
          errs() << "\nStripped: ";
          Type *t = stripPointer(type);
          assert(t && "parameter must be a pointer.");
          t -> print(errs());
          errs() << "\n";
          IRBuilderBase::LowRISCMemoryTag shouldTag = shouldTagTypeOfWord(t);
          if(shouldTag == IRBuilderBase::TAG_NORMAL && 
             isInt8Pointer(t) && isa<Instruction>(ptr)) {
            errs() << "Hmmm....\n";
            assert(isa<Instruction>(ptr));
            shouldTag = shouldTagBitCastInstruction(ptr);
          }
          if(shouldTag != IRBuilderBase::TAG_NORMAL) {
            errs() << "Should tag the load: " << shouldTag << "\n";
            createCheckTagged(instructions, it, ptr, M, shouldTag);
            doneSomething = true;
          }
        }
      }
      for(std::vector<Instruction*>::iterator it = toDelete.begin(); it != toDelete.end(); it++) {
        errs() << "Deleting instruction " << *it << "\n";
        (*it)->eraseFromParent();
      }
      getFunctionCheckTagged();
      return doneSomething;
    }

    /** Common idiom in generated IR: Bitcast before store. */
    IRBuilderBase::LowRISCMemoryTag shouldTagBitCastInstruction(Value *ptr) {
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
        Type *stripped = stripPointer(type);
        if(stripped)
          return shouldTagTypeOfWord(stripped);
      }
      return IRBuilderBase::TAG_NORMAL;
    }
    
    // FIXME lots of duplication getting function-level-or-higher globals here
    // FIXME duplication with IRBuilder.
    
    /* Insert stag(ptr, TAG_CLEAN) after the current instruction. */
    void createSTag(BasicBlock::InstListType& instructions, 
                         BasicBlock::InstListType::iterator it, Value *Ptr, Module *M,
                         IRBuilderBase::LowRISCMemoryTag tag) {
      assert(isa<PointerType>(Ptr->getType()) &&
       "stag only applies to pointers.");
      LLVMContext &Context = M->getContext();
      Ptr = getCastedInt8PtrValue(Context, instructions, it, Ptr); // FIXME consider int64 ptr
      Value *TagValue = getInt64(tag, Context);
      Value *Ops[] = { TagValue, Ptr };
      Value *TheFn = Intrinsic::getDeclaration(M, Intrinsic::riscv_stag);
      CallInst *CI = CallInst::Create(TheFn, Ops, "");
      // Set tag afterwards.
      instructions.insertAfter(it, CI);
    }

    void createStoreAndSetTag(BasicBlock::InstListType& instructions, 
                         BasicBlock::InstListType::iterator it, Value *DataValue, 
                         Value *Ptr, Module *M, IRBuilderBase::LowRISCMemoryTag tag) {
      assert(isa<PointerType>(Ptr->getType()) &&
       "stag only applies to pointers.");
      LLVMContext &Context = M->getContext();
      Ptr = getCastedInt8PtrValue(Context, instructions, it, Ptr); // FIXME consider int64 ptr
      // FIXME will only work with pointers, change if move to somewhere more generic.
      DataValue = CreatePtrToInt(DataValue, Type::getInt64Ty(Context), Context, instructions, it);
      Value *TagValue = getInt64(tag, Context);
      Value *Ops[] = { DataValue, TagValue, Ptr };
      Value *TheFn = Intrinsic::getDeclaration(M, Intrinsic::riscv_store_tagged);
      CallInst *CI = CallInst::Create(TheFn, Ops, "");
      instructions.insertAfter(it, CI);
    }

    /* Call __llvm_riscv_check_tagged before the current instruction */
    void createCheckTagged(BasicBlock::InstListType& instructions, 
                         BasicBlock::InstListType::iterator it, Value *Ptr, Module *M,
                         IRBuilderBase::LowRISCMemoryTag tag) {
      assert(isa<PointerType>(Ptr->getType()) &&
       "stag only applies to pointers.");
      LLVMContext &Context = M->getContext();
      Ptr = getCastedInt8PtrValue(Context, instructions, it, Ptr); // FIXME consider int64 ptr
      Value *Ops[] = { Ptr, getInt64(tag, Context) };
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
    
    Value *CreatePtrToInt(Value *V, Type *DestTy,
                         LLVMContext &Context,
                         BasicBlock::InstListType& instructions, 
                         BasicBlock::InstListType::iterator it,
                         const Twine &Name = "") {
      return CreateCast(Instruction::PtrToInt, V, DestTy, instructions, it, Name);
    }

    Value *CreateCast(Instruction::CastOps Op, Value *V, Type *DestTy,
                      BasicBlock::InstListType& instructions, 
                      BasicBlock::InstListType::iterator it,
                      const Twine &Name = "") {
      if (V->getType() == DestTy)
        return V;
      if (Constant *VC = dyn_cast<Constant>(V))
        return Folder.CreateCast(Op, VC, DestTy);
      Instruction *Inst = CastInst::Create(Op, V, DestTy);
      instructions.insert(it, Inst);
      return Inst;
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

