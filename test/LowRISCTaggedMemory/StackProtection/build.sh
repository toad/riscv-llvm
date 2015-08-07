#!/bin/bash
# FIXME Build it all with clang when it works! :(
rm -f *.s *.ll *.riscv
INCLUDES_NEWLIB="-I. -I $RISCV/riscv64-unknown-elf/include/"
INCLUDES_GLIBC="-I $RISCV/sysroot/usr/include/"
for main in main*.c; do
	for build in gcc clang gcc-linux clang-linux
	do
	rm *.s
	case "$build" in
		gcc)
			echo Building $main with gcc
			riscv64-unknown-elf-gcc -fpermissive -O0 -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S -fno-stack-protector $main -o ${main}.s || exit 1
			;;
		"clang"|"clang-linux")
			echo Building $main with clang
			if test "$build" = "clang"; then
				INCLUDES="$INCLUDES_NEWLIB"
			else
				INCLUDES="$INCLUDES_GLIBC"
			fi
			if ! clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/ -I $RISCV/riscv64-unknown-elf/include/ -I $RISCV/riscv64-unknown-elf/include/c++/4.9.2/riscv64-unknown-elf/ -S -fno-stack-protector $main -o ${main}.s; then echo Failed to build $main with $build; break; fi
			;;
		gcc-linux)
			riscv64-unknown-linux-gnu-gcc -fno-stack-protector -O0 -static -S -o main.s $main || exit 4
			;;
		*)
			echo "./build.sh BUILDTYPE"
			echo "Where BUILDTYPE is:"
			echo "  clang: Test LLVM with tags"
			echo "  gcc: Test GCC without tags"
			echo "  gcc-linux: Build test for booted Linux using GCC"
			echo "  clang-linux: Build test for booted Linux using Clang with tags"
			exit 1
			;;
	esac
	case "$build" in
		"clang"|"gcc")
			echo Assembling and linking with gcc
			riscv64-unknown-elf-g++ -O0 -o test-${main}.${build}.riscv ${main}.s || exit 3
			;;
		"clang-linux"|"gcc-linux")
			riscv64-unknown-linux-gnu-gcc -static -o test-${main}.${build}.riscv *.s || exit 4
			;;
	esac
	echo Successfully built test-${main}.${build}.riscv
	done
done
