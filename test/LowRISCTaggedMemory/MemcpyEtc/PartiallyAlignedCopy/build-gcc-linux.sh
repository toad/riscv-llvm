#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s *.ll *.riscv *.bc
x=main
LENGTH=${1:-3}
riscv64-unknown-linux-gnu-gcc -static -O0 -I . -I $RISCV/riscv64-unknown-elf/include/ ${x}.c -o $x.gcc-linux.riscv-linux -DLENGTH=$LENGTH -D__TAGGED_MEMORY__ || exit 5
