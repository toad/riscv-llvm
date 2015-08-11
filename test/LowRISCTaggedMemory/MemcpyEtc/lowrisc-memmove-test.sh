#!/bin/bash
OWD=$(pwd)
BUILDWITH=${1:-clang}
TMP=$(mktemp -d)
cd $TMP
for TEST in plain without-tags; do
cat $TOP/lowrisc-chip/riscv-tools/riscv-gnu-toolchain/build/src/newlib/newlib/testsuite/newlib.string/memmove1.c | sed "s/exit *(0);/{ printf(\"Success\\\\n\"); exit(0); }/" | sed "s/abort *();/{printf(\"Failure.\\\\n\"); abort();};/" | grep -v "  int errors = 0;" | perl -pe 's/(main .void.)\n/\1/igs' | sed "s/\(main .void.\) *{/\\1 {\n  printf(\"Starting test\\\\n\");/" > memmove1.c
if test "$TEST" == "without-tags"; then echo Testing with __riscv_memmove_no_tags; sed -i "s/ memmove / __riscv_memmove_no_tags /" memmove1.c; (echo "#include \"sys/platform/tag.h\""; cat memmove1.c) > memmove2.c; mv memmove2.c memmove1.c; fi
case "$BUILDWITH" in
	"clang")
		echo Building with Clang
		clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC -I $RISCV/riscv64-unknown-elf/include/ -S memmove1.c -o memmove1.s
	;;
	"gcc")
		echo Building with GCC
		riscv64-unknown-elf-gcc -O0 -I -I $RISCV/riscv64-unknown-elf/include/ -S memmove1.c -o memmove1.s
	;;
	"gcc-linux")
		echo Building with GCC for booted Linux
		riscv64-unknown-linux-gnu-gcc -O0 -I $RISCV/riscv-linux/include/ -static memmove1.c -o memmove1.riscv.linux -fno-builtin-memmove -fno-builtin-memcpy
		echo Now run memmove1.riscv.linux in a booted kernel
		cp memmove1.riscv.linux $OWD
		cd $OWD
		rm -Rf $TMP
		exit 0
	;;
	"clang-linux")
		echo Building with Clang for booted Linux
		INCLUDES_GLIBC="-I $RISCV/sysroot/usr/include/"
		if ! clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC $INCLUDES_GLIBC -S memmove1.c -emit-llvm -o memmove1.ll; then echo Failed to build; exit 3; fi
		if ! opt -load $TOP/riscv-llvm/build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < memmove1.ll > memmove1.opt.bc; then echo Failed to optimise; exit 3; fi
		if ! llc -use-init-array -filetype=asm -march=riscv -mcpu=LowRISC memmove1.opt.bc -o memmove1.s; then echo Failed to convert optimised code to assembler; exit 3; fi
		if ! riscv64-unknown-linux-gnu-gcc -static -o memmove1.riscv.linux memmove1.s; then echo Failed to assemble and link with gcc; exit 3; fi
		cp memmove1.riscv.linux $OWD
		rm -Rf $TMP
		echo Now run memmove1.riscv.linux in a booted kernel
		exit 0
		;;
	*)
		exit 2
esac
riscv64-unknown-elf-gcc memmove1.s -o memmove1.riscv
if spike pk memmove1.riscv | grep Success 2>&1; then echo Successful test; else echo Failure; spike pk memmove1.riscv; exit 1; fi
done
cd $OWD
rm -Rf $TMP
