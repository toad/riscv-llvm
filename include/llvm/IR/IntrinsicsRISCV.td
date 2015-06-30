//===- IntrinsicsX86.td - Defines RISC-V intrinsics -----------*- tablegen -*-===//
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

let TargetPrefix = "lowrisc" in {
  // FIXME can we talk about alignment? Want to be a pointer to a 64-bit type?
  // FIXME Request review on choice of Intr*Mem!
  def stag : Intrinsic<[],
                       [llvm_anyptr_ty, llvm_i8_ty],
                       [IntrReadWriteArgMem, NoCapture<0>]>;
  def ltag : Intrinsic<[llvm_i8_ty],
                       [llvm_anyptr_ty],
                       [IntrReadWriteArgMem, NoCapture<0>]>;
}
