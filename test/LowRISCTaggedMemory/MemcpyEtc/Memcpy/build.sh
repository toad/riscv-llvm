#!/bin/bash
build=${1:-clang}
rm -f *.s *.ll *.riscv *.bc
ARRAY_SIZE=${1:-10000}
for x in object tag; do
	case "$build" in
	"clang")
		echo Building ${x}.c with clang
		clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I. -I $RISCV/riscv64-unknown-elf/include/ -S ${x}.c -emit-llvm -DARRAY_SIZE=$ARRAY_SIZE -o ${x}.ll || exit 2
		opt -load $TOP/riscv-llvm/build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${x}.ll > ${x}.opt.bc || exit 3
		llc -filetype=asm -march=riscv -mcpu=LowRISC ${x}.opt.bc -o ${x}.opt.s || exit 4
		;;
	"gcc")
		riscv64-unknown-elf-gcc -O0 -I $RISCV/riscv-linux/include/ -S ${x}.c -DARRAY_SIZE=$ARRAY_SIZE -o ${x}.s || exit 2
		;;
	*)
		exit 5
	esac
done
for main in main*.c; do
	rm -f main*.s
	case "$build" in
	"gcc")
		echo Building $main with gcc
		riscv64-unknown-elf-gcc -fpermissive -O0 -I $RISCV/riscv64-unknown-elf/include/ -S -DARRAY_SIZE=$ARRAY_SIZE $main -o ${main}.s || exit 1
	;;
	"clang")
		echo Building $main with clang
		if ! clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I. -I $RISCV/riscv64-unknown-elf/include/ -S -DARRAY_SIZE=$ARRAY_SIZE $main -emit-llvm -o ${main}.ll; then echo Failed to build $main with $build; break; fi
		if ! opt -load $TOP/riscv-llvm/build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${main}.ll > ${main}.opt.bc ; then echo Failed to optimise $main; break; fi
		if ! llc -filetype=asm -march=riscv -mcpu=LowRISC ${main}.opt.bc -o ${main}.opt.s; then echo Failed to convert optimised $main to assembler; break; fi
		;;
	*)
		exit 5
		;;
	esac
	echo Assembling and linking with gcc
	riscv64-unknown-elf-g++ -o test-${main}.${build}.riscv *.s || exit 3
	echo Successfully built test-${main}.${build}.riscv
done
