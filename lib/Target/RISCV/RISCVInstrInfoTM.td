//==- RISCVInstrInfoTM.td - Tagged memory RISCV/lowRISC instructions --*- tblgen-*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// I-type instructions, similar to lwu/swu
// Rather confusing documentation.

// Surely this is a B-type? I guess it doesn't matter as we are producing assembler, not binary?
// Treat as InstStore
// FIXME add HasTM to the required list. At the moment that causes it to fail to match somehow.
def STAG : InstStore<"stag", 0b1010111, 0b001, int_riscv_stag, GR64, mem64>, Requires<[IsRV64]>;

// InstLoad actually fits this best...
//def LTAG : InstLoad<"ltag", 0b1010111, 0b000, int_riscv_ltag, GR64, mem64>, Requires<[IsRV64]>;

