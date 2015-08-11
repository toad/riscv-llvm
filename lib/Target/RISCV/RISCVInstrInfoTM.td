//==- RISCVInstrInfoTM.td - Tagged memory RISCV/lowRISC instructions --*- tblgen-*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Treat as InstStore
def STAG : InstStore<"stag", 0b1010111, 0b001, int_riscv_stag, GR64, mem64>, Requires<[IsRV64, HasTM]>;

// Treat as InstLoad
def LTAG : InstLoad<"ltag", 0b1010111, 0b000, int_riscv_ltag, GR64, mem64>, Requires<[IsRV64, HasTM]>;

// rs1.value, rs2.value -> rd.value, rd.tag

// FIXME Copied here because it has no operations, and we can't encode that using InstI.
// FIXME Add an intrinsic to tag a value when we have a move operation. Then we can clean this up.

def WRT : InstRISCV<4, (outs GR64:$dst), (ins GR64:$src2, GR64:$src1), 
                "wrt\t$dst, $src1, $src2", 
                []> {
  field bits<32> Inst;

  bits<5> RD;
  bits<5> RS1;
  bits<5> RS2;

  let Inst{31-27} = RD;
  let Inst{26-22} = RS1;
  let Inst{21-17} = RS2;
  let Inst{16- 7} = 0; // Will change!
  let Inst{6 - 0} = 6; // Will change!
}

// Write a tagged value to memory
def : Pat<(int_riscv_store_tagged GR64Bit:$val, GR64Bit:$tag, GR64Bit:$addr), 
          (SD (WRT GR64Bit:$val, GR64Bit:$tag), GR64Bit:$addr, 0)>;
