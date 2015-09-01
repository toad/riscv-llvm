#!/bin/bash
rm -f *.ll *.bc *.riscv *.s
for compiler in clang gcc-force-tags gcc-no-tags
do
	./build.sh $compiler || exit 1
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
	# Exploits. These should call the evil function with GCC, but exit with Clang.
	for x in exploit-*.c
	do
		if ! test "$compiler" == "gcc-no-tags"; then
			if ! spike pk test-${x}.${compiler}.riscv | grep "Called evil function"
			then
				if ! test "$compiler" == "clang"; then
					echo "Test with protection turned off did not run evil code?!: $x with compiler $compiler"
					echo "This means there is a bug in the test"
					exit 2
				else
					echo "Clang protection prevented running evil code on $x"
				fi
			else
				if test "$compiler" == "clang"; then
					echo "Clang protection failed, ran evil code on $x"
					exit 3
				else
					echo "Test with protection turned off ran evil code $x with compiler $compiler as expected"
				fi
			fi
		fi
		echo "Test $x succeeded with $compiler - failed as expected"
	done
	# Failing tests. These should work with GCC but fail with Clang.
	if ls fail-*.c; then
	for x in fail-*.c
	do
		if ! spike pk test-${x}.${compiler}.riscv | grep "Success"; then
			if ! test "$compiler" == "clang"; then
				echo "Test with protection turned off did not work: $x with compiler $compiler"
				echo "This means there is a bug in the test"
				exit 2
			else
				echo "Clang test $x failed as expected"
			fi
		fi
		echo "Test $x succeeded with $compiler - failed as expected"
	done
	fi
	echo "All tests succeeded for $compiler"
done
compiler=gcc-fail-tags
echo "Testing $compiler"
	./build.sh $compiler
	for x in main-*.c exploit-*.c
	do
		# main-simple.c just checks that tags work, so don't build for gcc-fail-tags
		if test "$x" == "main-simple.c"; then continue; fi
		# These only make sense if we enable pointer/void* tagging. FIXME.
		if test "$x" == "main-ordinary-pointer.c"; then continue; fi
		if test "$x" == "main-void-pointer.c"; then continue; fi
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

