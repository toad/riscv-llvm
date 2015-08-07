#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s *.ll *.riscv
for x in Test SubclassTest; do
	echo Building ${x}.cc with clang
	clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S ${x}.cc -emit-llvm -o ${x}.ll -fno-exceptions -DCHECK_TAGS || exit 2
	opt -load ../../../build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${x}.ll > ${x}.opt.bc || exit 3
	llvm-dis ${x}.opt.bc > ${x}.opt.ll
	llc -use-init-array -filetype=asm -march=riscv -mcpu=LowRISC ${x}.opt.bc -o ${x}.opt.s || exit 4
done
for main in main-*.cc; do
	for build in gcc clang gcc-linux
	do
		rm -f main*.s
		case "$build" in
		"gcc")
			echo Building $main with gcc
			riscv64-unknown-elf-gcc -fpermissive -O0 -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S $main -o ${main}.s || exit 1
			;;
		"clang")
			echo Building $main with clang
			if ! clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S $main -emit-llvm -o ${main}.ll -fno-exceptions -DCHECK_TAGS; then echo Failed to build $main with $build; break; fi
			if ! opt -load ../../../build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${main}.ll > ${main}.opt.bc ; then echo Failed to optimise $main; break; fi
		        llvm-dis ${main}.opt.bc > ${main}.opt.ll
			if ! llc -use-init-array -filetype=asm -march=riscv -mcpu=LowRISC ${main}.opt.bc -o ${main}.opt.s; then echo Failed to convert optimised $main to assembler; break; fi
			;;
		esac
		case "$build" in
		"gcc-linux")
			riscv64-unknown-linux-gnu-g++ -fno-stack-protector -O0 -static -o test-${main}.${build}.riscv $main Test.cc SubclassTest.cc || exit 4
			;;
		*)
			echo Assembling and linking with gcc
			riscv64-unknown-elf-g++ -o test-${main}.${build}.riscv *.s || exit 3
		esac
		echo Successfully built test-${main}.${build}.riscv
	done
done
