#!/bin/bash
rm -f *.ll *.bc *.riscv *.s
for compiler in clang gcc-force-tags gcc-no-tags
do
	./build.sh $compiler
	for x in main-*.c
	do
		# main-simple.c just checks that tags work, so don't build for gcc-no-tags
		if test "$compiler" == "gcc-no-tags" && test "$x" == "main-simple.c"; then continue; fi
		if ! spike pk test-${x}.${compiler}.riscv | grep Success
		then
			echo "Failed test $x with compiler $compiler"
			exit 1
		fi
		echo "Test $x succeeded with $compiler"
	done
	# Don't build failing tests for gcc-no-tags
	for x in fail-*.c
	do
		if test "$compiler" == "gcc-no-tags"; then
			if ! spike pk test-${x}.${compiler}.riscv | grep "Called evil function"
			then
				echo "Test with protection turned off did not run evil code?!: $x with compiler $compiler"
				echo "This means there is a bug in the test"
				exit 2
			else
				echo "Test with $compiler ran evil code as expected"
				echo "With clang it should fail"
			fi
		else
			if spike pk test-${x}.${compiler}.riscv | grep Success
			then
				echo "Test succeeded but should fail: $x with compiler $compiler"
				echo "This means protection isn't working"
				exit 2
			fi
		fi
		echo "Test $x succeeded with $compiler (failed as expected)"
	done
	echo "All tests succeeded for $compiler"
done
compiler=gcc-fail-tags
echo "Testing $compiler"
	./build.sh $compiler
	for x in main-*.c fail-*.c
	do
		# main-simple.c just checks that tags work, so don't build for gcc-fail-tags
		if test "$x" == "main-simple.c"; then continue; fi
		if ! spike pk test-${x}.${compiler}.riscv | grep "assertion.*failed"
		then
			echo "Test succeeded but should fail: $x"
			echo "This means there's a bug in the test"
			exit 1
		fi
		echo "Test $x succeeded with $compiler (failed as expected)"
	done
	echo "All tests succeeded for $compiler"
echo "All tests succeeded"

