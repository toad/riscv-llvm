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
  def int_riscv_stag : Intrinsic<[],
                       [llvm_i64_ty, llvm_ptr_ty],
                       [IntrReadWriteArgMem, NoCapture<1>]>;
  def int_riscv_ltag : Intrinsic<[llvm_i64_ty],
                       [llvm_ptr_ty],
                       [IntrReadWriteArgMem, NoCapture<0>]>;
}
