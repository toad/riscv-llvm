#!/bin/bash
VARIABLES=""
INCLUDES_NEWLIB="-I. -I $RISCV/riscv64-unknown-elf/include/"
INCLUDES_GLIBC="-I $RISCV/sysroot/usr/include/"
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
		VARIABLES="$VARIABLES -DFAKE_TAGS -D__TAGGED_MEMORY__"
		;;
	gcc-fail-tags)
		# Build with GCC, and check for tags.
		# Should fail because GCC doesn't set them.
		build=gcc
		VARIABLES="$VARIABLES -D__TAGGED_MEMORY__"
		;;
	gcc-linux)
		# Build with GCC for a booted Linux system, with fake tags.
		# Should succeed.
		# Harder to run.
		build=gcc-linux
		VARIABLES="$VARIABLES -DFAKE_TAGS -D__TAGGED_MEMORY__"
		;;
	clang-linux)
		# Build with Clang for a booted Linux system with full tag support
		build=clang-linux
		;;
	*)
		echo "./build.sh BUILDTYPE"
		echo "Where BUILDTYPE is:"
		echo "  clang: Test LLVM with tags"
		echo "  gcc-no-tags: Test GCC without tags"
		echo "  gcc-force-tags: Test GCC with setting tags manually"
		echo "  gcc-fail-tags: Test GCC with tags (should fail)"
		echo "  gcc-linux: Build test for booted Linux using GCC with fake tags (should succeed)"
		echo "  clang-linux: Build test for booted Linux using Clang with tags"
		exit 1
esac
rm -f *.s *.ll *.bc
for main in $(ls main*.c fail*.c exploit*.c); do
	rm -f main*.s fail*.s exploit*.s
	case "$build" in
	"gcc")
		echo Building $main with gcc
		riscv64-unknown-elf-gcc -fpermissive -O0 -I $RISCV/riscv64-unknown-elf/include/ -S $VARIABLES $main -o ${main}.s || exit 1
		;;
	"gcc-linux")
		# Do nothing.
		;;
	"clang"|"clang-linux")
		echo Building $main with clang
		if test "$build" = "clang"; then
			INCLUDES="$INCLUDES_NEWLIB"
		else
			INCLUDES="$INCLUDES_GLIBC"
		fi
		if ! clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC $INCLUDES -S $VARIABLES $main -emit-llvm -o ${main}.ll; then echo Failed to build $main with $build; break; fi
		OPTS="-O2 -scalar-evolution -loop-reduce"
		if echo "${main}" | grep "^fail-" || echo "${main}" | grep "^exploit-" || test "${main}" == "main-pointer-swap.c"; then
			# Testing undefined behaviour
			# Behaviour will depend on optimisation level
			OPTS=""
		fi
		if ! opt -load $TOP/riscv-llvm/build/Debug+Asserts/lib/LLVMTagCodePointers.so $OPTS -tag-code-pointers < ${main}.ll > ${main}.opt.bc ; then echo Failed to optimise $main; break; fi
		if ! llc -use-init-array -filetype=asm -march=riscv -mcpu=LowRISC ${main}.opt.bc -o ${main}.opt.s; then echo Failed to convert optimised $main to assembler; break; fi
		;;
	*)
		exit 5
		;;
	esac
	echo Assembling and linking with gcc
	if test "$build" == "gcc-linux"
	then
		riscv64-unknown-linux-gnu-gcc -static -o test-${main}.$1.riscv-linux -O0 -I $RISCV/riscv-linux/include/ $VARIABLES $main || exit 3
	elif test "$build" == "clang-linux"
	then
		riscv64-unknown-linux-gnu-gcc -static -o test-${main}.$1.riscv-linux -I $RISCV/riscv-linux/include/ $VARIABLES *.s || exit 3
	else
		riscv64-unknown-elf-gcc -o test-${main}.$1.riscv *.s || exit 3
	fi
	echo Successfully built test-${main}.$1.riscv
done
