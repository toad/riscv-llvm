#!/bin/bash
for x in $(seq 0 100) $(seq 2000 2100)
do
	echo Testing partially aligned copy with size $x
	./build.sh $x > /dev/null 2>&1 || exit 1
	if ! spike pk main.riscv | grep Success > /dev/null;
	then
		spike pk main.riscv
		echo Failed with Clang with size $x
		exit 2
	fi
done
