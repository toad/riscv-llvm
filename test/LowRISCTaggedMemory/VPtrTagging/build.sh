#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s
for x in Test SubclassTest; do
	echo Building ${x}.cc with clang
	clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S ${x}.cc || exit 2
done
for main in main-*.cc; do
	for build in gcc clang
	do
		rm -f main*.s
		if test "$build" = "gcc"; then
			echo Building $main with gcc
			riscv64-unknown-elf-gcc -fpermissive -O0 -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S $main || exit 1
		else
			echo Building $main with clang
			if ! clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S $main; then echo Failed to build $main with $build; break; fi
		fi
		echo Assembling and linking with gcc
		riscv64-unknown-elf-g++ -o test-${main}.${build}.riscv Test.s SubclassTest.s main*.s || exit 3
		echo Successfully built test-${main}.${build}.riscv
	done
done
