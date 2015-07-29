#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s *.ll *.riscv *.bc
x=main
LENGTH=${1:-3}
clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/ -S ${x}.c -emit-llvm -DLENGTH=$LENGTH -o ${x}.ll || exit 2
opt -load $TOP/riscv-llvm/build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${x}.ll > ${x}.opt.bc || exit 3
llc -filetype=asm -march=riscv -mcpu=LowRISC ${x}.opt.bc -o ${x}.opt.s || exit 4
riscv64-unknown-elf-g++ -o $x.riscv *.s || exit 5
