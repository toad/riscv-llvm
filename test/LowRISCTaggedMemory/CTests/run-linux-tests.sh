#!/bin/bash
if ! ./build.sh gcc-linux; then echo Failed to build for GCC; exit 1; fi
if ! ./build.sh clang-linux; then echo Failed to build for Linux; exit 1; fi
for x in main*.c
do
	if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-${x}.gcc-linux.riscv-linux
	then
		echo Test failed: $x
		exit 2
	fi
	echo Test $x succeeded on GCC-linux as expected
	if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-${x}.clang-linux.riscv-linux
	then
		echo Test failed: $x
		exit 2
	fi
	echo Test $x succeeded on Clang-linux as expected
done
for x in fail*.c
do
	echo Test $x should fail with gcc...
	if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-${x}.gcc-linux.riscv-linux | grep "Called evil function, failure"
	then
		echo Test should have failed but succeeded: $x
		exit 3
	fi
	echo Test $x failed with gcc as expected
	echo Test $x should abort with clang...
	if $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-${x}.clang-linux.riscv-linux | grep "Called evil function, failure"
	then
		echo Clang protection failed: $x
		exit 3
	fi
	echo Test $x aborted as expected with clang
done
echo All C tests successful
