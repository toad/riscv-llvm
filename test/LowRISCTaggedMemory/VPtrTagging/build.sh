#!/bin/bash
rm -f *.riscv
INCLUDES_NEWLIB="-I. -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/5.2.0/ -I $RISCV/riscv64-unknown-elf/include/c++/5.2.0/riscv64-unknown-elf/"
INCLUDES_GLIBC="-I $RISCV/sysroot/usr/include/ -I $RISCV/riscv64-unknown-linux-gnu/include/c++/5.2.0/ -I $RISCV/riscv64-unknown-linux-gnu/include/c++/5.2.0/riscv64-unknown-linux-gnu"
for build in gcc clang gcc-linux clang-linux
do
echo Building for $build
rm -f *.s *.ll
case "$build" in
	"gcc"|"clang")
		INCLUDES="$INCLUDES_NEWLIB"
                riscv64-unknown-elf-gcc -S fail-error.newlib.c
		;;
	"gcc-linux"|"clang-linux")
		INCLUDES="$INCLUDES_GLIBC"
                riscv64-unknown-linux-gnu-gcc -S fail-error.glibc.c
		;;
esac
for x in Test SubclassTest; do
	case $build in
	"gcc")
		riscv64-unknown-elf-g++ -S $INCLUDES ${x}.cc -o ${x}.s -fno-exceptions -fno-stack-protector
		;;
	"gcc-linux")
		riscv64-unknown-linux-gnu-g++ -S $INCLUDES ${x}.cc -o ${x}.s -fno-exceptions -fno-stack-protector
		;;
	"clang"|"clang-linux")
		echo Building ${x}.cc with clang
		clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC $INCLUDES -S ${x}.cc -emit-llvm -o ${x}.ll -fno-exceptions -DCHECK_TAGS || exit 2
		opt -load ../../../build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers ${x}.ll -o ${x}.opt.bc || exit 3
		llvm-dis ${x}.opt.bc > ${x}.opt.ll
		llc -use-init-array -filetype=asm -march=riscv -mcpu=LowRISC ${x}.opt.bc -o ${x}.opt.s || exit 4
		;;
	esac
done
for main in main-*.cc; do
		rm -f main*.s
		case "$build" in
		"gcc")
			echo Building $main with gcc
			riscv64-unknown-elf-gcc -fpermissive -O0 $INCLUDES -S $main -o ${main}.s || exit 1
			;;
		"gcc-linux")
			echo Building $main with gcc for linux
			riscv64-unknown-linux-gnu-gcc -fpermissive -O0 $INCLUDES -S $main -o ${main}.s || exit 1
			;;
		"clang"|"clang-linux")
			echo Building $main with clang
			if ! clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC $INCLUDES -S $main -emit-llvm -o ${main}.ll -fno-exceptions -DCHECK_TAGS; then echo Failed to build $main with $build; break; fi
			if ! opt -load ../../../build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers ${main}.ll -o ${main}.opt.bc ; then echo Failed to optimise $main; break; fi
		        llvm-dis ${main}.opt.bc > ${main}.opt.ll
			if ! llc -use-init-array -filetype=asm -march=riscv -mcpu=LowRISC ${main}.opt.bc -o ${main}.opt.s; then echo Failed to convert optimised $main to assembler; break; fi
			;;
		esac
		case "$build" in
		"clang-linux"|"gcc-linux")
			riscv64-unknown-linux-gnu-g++ -fno-stack-protector -fno-exceptions -O0 -static -o test-${main}.${build}.riscv *.s || exit 5
			;;
		*)
			echo Assembling and linking with gcc
			riscv64-unknown-elf-g++ -o test-${main}.${build}.riscv *.s || exit 3
			;;
		esac
		echo Successfully built test-${main}.${build}.riscv
	done
done
