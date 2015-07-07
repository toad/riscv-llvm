#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s
for x in Test SubclassTest; do
	echo Building ${x}.cc with clang
	clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S ${x}.cc || exit 2
done
for main in main-*.cc; do
	case $main in
		main-vtable-replacement.cc)
			build=gcc
			;;
		main-read-only-vptr.cc)
			build=gcc
			;;
		*)
			build=clang
			;;
	esac
	rm -f main*.s
	if test "$build" = "gcc"; then
		echo Building $main with gcc
		riscv64-unknown-elf-gcc -fpermissive -O0 -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S $main || exit 1
	else
		echo Building $main with clang
		clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S $main || exit 1
	fi
	echo Assembling and linking with gcc
	riscv64-unknown-elf-g++ -o test-${main}.riscv Test.s SubclassTest.s main*.s || exit 3
	echo Success!
done
