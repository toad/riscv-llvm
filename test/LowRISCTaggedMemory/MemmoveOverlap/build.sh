#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s *.ll *.riscv *.bc
x=main
clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I -I $RISCV/riscv64-unknown-elf/include/ -S ${x}.c -emit-llvm -DARRAY_SIZE=$ARRAY_SIZE -o ${x}.ll || exit 2
opt -load ../../../build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${x}.ll > ${x}.opt.bc || exit 3
llc -filetype=asm -march=riscv -mcpu=LowRISC ${x}.opt.bc -o ${x}.opt.s || exit 4
riscv64-unknown-elf-g++ -o $x.riscv *.s || exit 5
