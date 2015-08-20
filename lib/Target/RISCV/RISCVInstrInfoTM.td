//==- RISCVInstrInfoTM.td - Tagged memory RISCV/lowRISC instructions --*- tblgen-*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// FIXME Consider adding tags as separate registers.
// Register tag operations could then be expressed directly.
// Memory ops are less clear-cut. We could use an address space but because of side-effects it might not do what we want?
// Probably still need some custom lowering?

// Treat as InstStore
def STAG : InstStore<"stag", 0b1010111, 0b001, int_riscv_stag, GR64, mem64>, Requires<[IsRV64, HasTM]>;

// Treat as InstLoad
def LTAG : InstLoad<"ltag", 0b1010111, 0b000, int_riscv_ltag, GR64, mem64>, Requires<[IsRV64, HasTM]>;

// Note that there are no intrinsics to set tags on values.
// The main reason for this is that most of the tags have side-effects in memory, so e.g. register
// spilling would be difficult.

// Atomically load, check tag and propagate tag into register.
let mayLoad = 1 in
def LDCT : InstLoadNONE<"ldct", 0b1010111, 0b100, GR64, mem64>, Requires<[IsRV64, HasTM]>;

// Atomically check existing tag, store both value and tag from register.
let mayStore = 1 in
def SDCT : InstStoreNONE<"sdct", 0b1010111, 0b101, GR64, mem64>, Requires<[IsRV64, HasTM]>;

// rs1.value, rs2.value -> rd.value, rd.tag

// FIXME Copied here because it has no operations, and we can't encode that using InstI.
// FIXME Add an intrinsic to tag a value when we have a move operation. Then we can clean this up.

def WRT : InstRISCV<4, (outs GR64:$dst), (ins GR64:$src2, GR64:$src1), 
                "wrt\t$dst, $src2, $src1", 
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

def RDT : InstRISCV<4, (outs GR64:$dst), (ins GR64:$src),
                    "rdt\t$dst, $src",
                    []> {
  field bits<32> Inst;

  bits<5> RD;
  bits<5> RS1;

  let Inst{31-27} = RD;
  let Inst{26-22} = RS1;
  let Inst{21-17} = 0;
  let Inst{16- 7} = 0; // Will change!
  let Inst{6 - 0} = 7; // Will change!
}

// Post-RA pseudo's.
// Spilling tagged values is problematic, so do this post-RA.

let mayStore = 1 in
def STORE_TAGGED : Pseudo<(outs), (ins GR64:$val, GR64:$tag, GR64:$addr), 
                          [(int_riscv_store_tagged GR64:$val, GR64:$tag, GR64:$addr)]>;

let mayLoad = 1 in
def LOAD_TAGGED : Pseudo<(outs GR64:$val, GR64:$tag), (ins GR64:$addr),
                          []>; // Will be emitted by custom lowering code.

