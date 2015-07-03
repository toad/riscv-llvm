#!/bin/bash
# FIXME Build it all with clang when it works! :(
echo Building main.cc with gcc
riscv64-unknown-elf-gcc -O0 -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S main.cc || exit 1
for x in Test SubclassTest; do 
echo Building ${x}.cc with clang
clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S ${x}.cc || exit 2
done
echo Assembling and linking with gcc
riscv64-unknown-elf-g++ -o test.riscv *.s || exit 3
echo Success!
