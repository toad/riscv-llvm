#!/bin/bash
TMP=$(mktemp -d)
cd $TMP
cat $TOP/lowrisc-chip/riscv-tools/riscv-gnu-toolchain/src/newlib/newlib/testsuite/newlib.string/memmove1.c | sed "s/exit *(0)/printf(\"Success\\\\n\"); exit(0)/" > memmove1.c
clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I -I $RISCV/riscv64-unknown-elf/include/ -S memmove1.c -o memmove1.s
riscv64-unknown-elf-gcc memmove1.s -o memmove1.riscv
if spike pk memmove1.riscv | grep Success 2>&1; then echo Successful test; rm -Rf $TMP; else echo Failure; spike pk memmove1.riscv; fi
