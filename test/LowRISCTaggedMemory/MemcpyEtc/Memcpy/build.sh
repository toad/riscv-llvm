#!/bin/bash
ARRAY_SIZE=${2:-10000}
VARIABLES="-DARRAY_SIZE=$ARRAY_SIZE"
case $1 in
	clang)
		# Build with Clang, with full tag support.
		# Should work.
		build=clang
		;;
	gcc-no-tags)
		# Build with GCC, and don't check tags.
		# Should work.
		build=gcc
		VARIABLES="$VARIABLES -DNO_TAGS"
		;;
	gcc-force-tags)
		# Build with GCC, set the tags manually.
		# Should work.
		build=gcc
		VARIABLES="$VARIABLES -DFAKE_TAGS"
		;;
	gcc-fail-tags)
		# Build with GCC, and check for tags.
		# Should fail because GCC doesn't set them.
		build=gcc
		VARIABLES="$VARIABLES"
		;;
	gcc-linux)
		# Build with GCC for a booted Linux system, with fake tags.
		# Should succeed.
		# Harder to run.
		build=gcc-linux
		VARIABLES="$VARIABLES -DFAKE_TAGS"
		;;
	*)
		echo "./build.sh BUILDTYPE"
		echo "Where BUILDTYPE is:"
		echo "  clang: Test LLVM with tags"
		echo "  gcc-no-tags: Test GCC without tags"
		echo "  gcc-force-tags: Test GCC with setting tags manually"
		echo "  gcc-fail-tags: Test GCC with tags (should fail)"
		echo "  gcc-linux: Build test for booted Linux using GCC with fake tags (should succeed)"
		exit 1
esac
rm -f *.s *.ll *.bc
for x in object tag; do
	case "$build" in
	"clang")
		echo Building ${x}.c with clang
		clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I. -I $RISCV/riscv64-unknown-elf/include/ -S ${x}.c -emit-llvm $VARIABLES -o ${x}.ll || exit 2
		opt -load $TOP/riscv-llvm/build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${x}.ll > ${x}.opt.bc || exit 3
		llc -filetype=asm -march=riscv -mcpu=LowRISC ${x}.opt.bc -o ${x}.opt.s || exit 4
		;;
	"gcc")
		riscv64-unknown-elf-gcc -O0 -I $RISCV/riscv64-unknown-elf/include/ -S ${x}.c $VARIABLES -o ${x}.s || exit 2
		;;
	"gcc-linux")
		# Do nothing.
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
		riscv64-unknown-elf-gcc -fpermissive -O0 -I $RISCV/riscv64-unknown-elf/include/ -S $VARIABLES $main -o ${main}.s || exit 1
		;;
	"gcc-linux")
		# Do nothing.
		;;
	"clang")
		echo Building $main with clang
		if ! clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I. -I $RISCV/riscv64-unknown-elf/include/ -S $VARIABLES $main -emit-llvm -o ${main}.ll; then echo Failed to build $main with $build; break; fi
		if ! opt -load $TOP/riscv-llvm/build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${main}.ll > ${main}.opt.bc ; then echo Failed to optimise $main; break; fi
		if ! llc -filetype=asm -march=riscv -mcpu=LowRISC ${main}.opt.bc -o ${main}.opt.s; then echo Failed to convert optimised $main to assembler; break; fi
		;;
	*)
		exit 5
		;;
	esac
	echo Assembling and linking with gcc
	if test "$build" == "gcc-linux"
	then
		riscv-linux-gcc -static -o test-${main}.$1.${ARRAY_SIZE}.riscv-linux -O0 -I $RISCV/riscv-linux/include/ $VARIABLES main.c object.c tag.c || exit 3
	else
		riscv64-unknown-elf-gcc -o test-${main}.$1.${ARRAY_SIZE}.riscv *.s || exit 3
	fi
	echo Successfully built test-${main}.$1.${ARRAY_SIZE}.riscv
done
