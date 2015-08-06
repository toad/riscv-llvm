#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s *.ll *.riscv *.bc
INCLUDES="-I $RISCV/sysroot/usr/include/"
x=main
LENGTH=${1:-3}
clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC $INCLUDES -S ${x}.c -o $x.ll -emit-llvm -DLENGTH=$LENGTH || exit 5
opt -load $TOP/riscv-llvm/build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${x}.ll > ${x}.opt.bc || exit 3
llc -filetype=asm -march=riscv -mcpu=LowRISC ${x}.opt.bc -o ${x}.opt.s || exit 4
riscv64-unknown-linux-gnu-gcc -static -o $x.clang-linux.riscv-linux *.s || exit 2

