#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s *.ll *.riscv *.bc
x=main
LENGTH=${1:-3}
riscv-linux-gcc -static -O0 -I . -I $RISCV/riscv64-unknown-elf/include/ ${x}.c -o $x.riscv-linux || exit 5
