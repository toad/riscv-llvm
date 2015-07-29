#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s *.ll *.riscv *.bc
x=main
LENGTH=${1:-3}
riscv64-unknown-elf-gcc -O0 -I . -I $RISCV/riscv64-unknown-elf/include/ ${x}.c -o $x.riscv -DLENGTH=$LENGTH -D__TAGGED_MEMORY__ || exit 5
