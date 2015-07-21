#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s *.ll *.riscv
for x in Test SubclassTest; do
	echo Building ${x}.cc with clang
	clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S ${x}.cc -emit-llvm -o ${x}.ll -fno-exceptions || exit 2
	opt -load ../../../build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${x}.ll > ${x}.opt.ll || exit 3
	llc -filetype=asm -march=riscv -mcpu=LowRISC ${x}.opt.ll -o ${x}.opt.s || exit 4
done
for main in main-*.cc; do
	for build in gcc clang
	do
		rm -f main*.s
		if test "$build" = "gcc"; then
			echo Building $main with gcc
			riscv64-unknown-elf-gcc -fpermissive -O0 -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S $main -o ${main}.s || exit 1
		else
			echo Building $main with clang
			if ! clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S $main -emit-llvm -o ${main}.ll -fno-exceptions; then echo Failed to build $main with $build; break; fi
			if ! opt -load ../../../build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${main}.ll > ${main}.opt.ll ; then echo Failed to optimise $main; break; fi
			if ! llc -filetype=asm -march=riscv -mcpu=LowRISC ${main}.opt.ll -o ${main}.opt.s; then echo Failed to convert optimised $main to assembler; break; fi
		fi
		echo Assembling and linking with gcc
		riscv64-unknown-elf-g++ -o test-${main}.${build}.riscv *.s || exit 3
		echo Successfully built test-${main}.${build}.riscv
	done
done
