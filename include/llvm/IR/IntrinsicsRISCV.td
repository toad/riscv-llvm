//===- IntrinsicsRISCV.td - Defines RISC-V intrinsics -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the RISC-V-specific intrinsics.
//
//===----------------------------------------------------------------------===//
// FIXME These are actually lowRISC specific intrinsics, but the compiler doesn't know the difference yet???
//

//===------------------- Tagged Memory Intrinsics --------------------===//

let TargetPrefix = "riscv" in {
  // FIXME can we talk about alignment? Want to be a pointer to a 64-bit type?
  // FIXME Request review on choice of Intr*Mem!
  // FIXME these should be i8 but that would mean more work in lowering
  def int_riscv_stag : Intrinsic<[],
                       [llvm_i64_ty, llvm_ptr_ty],
                       [IntrReadWriteArgMem, NoCapture<1>]>;
  def int_riscv_ltag : Intrinsic<[llvm_i64_ty],
                       [llvm_ptr_ty],
                       [IntrReadWriteArgMem, NoCapture<0>]>;
  def int_riscv_set_tm_trap_ld : Intrinsic<[], [llvm_i64_ty], []>;
  def int_riscv_set_tm_trap_sd : Intrinsic<[], [llvm_i64_ty], []>;
  // Atomically write a value with a specified tag to a memory location.
  // Arg 0 = value to write, arg 1 = tag, arg 2 = pointer to write to.
  def int_riscv_store_tagged : Intrinsic<[],
                                         [llvm_i64_ty, llvm_i64_ty, llvm_ptr_ty],
                                         [IntrReadWriteArgMem, NoCapture<2>]>;
  // Atomically load a 64-bit aligned value from memory with its tag.
  // FIXME Consider using anyptr here, then don't need so many bitcasts. Downside is might need a separate one for i64.
  // Would need to use LLVMMatchType and maybe LLVMPointerType, see overflow arithmetic intrinsics.
  // FIXME Use i64 ptr.
  // FIXME Change second return to be an i8.
  def int_riscv_load_tagged : Intrinsic<[llvm_i64_ty, llvm_i64_ty],
                                        [llvm_ptr_ty],
                                        [IntrReadArgMem, NoCapture<0>]>;
                                        
}
