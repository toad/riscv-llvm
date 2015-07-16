#!/bin/bash
for x in $(seq 0 1000)
do
	echo Testing non-overlapping memmove and memcpy with size $x
#	./build.sh gcc $x > /dev/null 2>&1 || exit 1
#	if ! spike pk test-main.c.gcc.riscv | grep Success > /dev/null;
#	then echo Failed with GCC with size $x; fi # Keep going
	./build.sh clang $x > /dev/null 2>&1 || exit 1
	if ! spike pk test-main.c.clang.riscv | grep Success > /dev/null;
	then
		spike pk test-main.c.clang.riscv
		echo Failed with Clang with size $x
		exit 2
	fi
done
