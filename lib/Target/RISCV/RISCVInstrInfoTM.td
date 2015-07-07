//==- RISCVInstrInfoTM.td - Tagged memory RISCV/lowRISC instructions --*- tblgen-*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Treat as InstStore
def STAG : InstStore<"stag", 0b1010111, 0b001, int_riscv_stag, GR64, mem64>, Requires<[IsRV64,HasTM]>;

// Treat as InstLoad
def LTAG : InstLoad<"ltag", 0b1010111, 0b000, int_riscv_ltag, GR64, mem64>, Requires<[IsRV64,HasTM]>;

