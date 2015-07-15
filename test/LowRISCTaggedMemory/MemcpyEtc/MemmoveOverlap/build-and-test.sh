#!/bin/bash
for x in $(seq 3 1000)
do
	echo Testing with size $x
	./build.sh $x > /dev/null 2>&1 || exit 1
	if ! spike pk main.riscv | grep Success > /dev/null;
	then
		spike pk main.riscv
		echo Failed with Clang with size $x
		exit 2
	fi
done
