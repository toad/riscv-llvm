#!/bin/bash
OWD=$(pwd)
BUILDWITH=${1:-clang}
TMP=$(mktemp -d)
cd $TMP
for TEST in plain without-tags; do
cat $TOP/lowrisc-chip/riscv-tools/riscv-gnu-toolchain/build/src/newlib/newlib/testsuite/newlib.string/memmove1.c | sed "s/exit *(0)/printf(\"Success\\\\n\"); exit(0)/" | perl -pe 's/(main .void.)\n/\1/igs' | sed "s/\(main .void.\) *{/\\1 {\n  printf(\"Starting test\\\\n\");/" > memmove1.c
if test "$TEST" == "without-tags"; then echo Testing with __riscv_memmove_no_tags; sed -i "s/ memmove / __riscv_memmove_no_tags /" memmove1.c; fi
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
	*)
		exit 2
esac
riscv64-unknown-elf-gcc memmove1.s -o memmove1.riscv
if spike pk memmove1.riscv | grep Success 2>&1; then echo Successful test; else echo Failure; spike pk memmove1.riscv; exit 1; fi
done
cd $OWD
rm -Rf $TMP
