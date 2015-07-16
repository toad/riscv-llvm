#!/bin/bash
# $1 = compiler type
# $2 = array size
test_success()
{
	if ! ./build.sh $1 $2 > /dev/null 2>&1
	then echo Could not build with $1 $2; exit 1; fi
	if ! spike pk test-main.c.$1.$2.riscv | grep Success > /dev/null 2>&1
	then
		echo Test failed for $1 $2
		spike pk test-main.c.$1.$2.riscv
		exit 2
	fi
}

test_failure() {
	if ! ./build.sh $1 $2 > /dev/null 2>&1
	then echo Could not build with $1 $2; exit 1; fi
	if spike pk test-main.c.$1.$2.riscv | grep Success > /dev/null 2>&1
	then
		echo "Test succeeded for $1 $2 should have failed"
		spike pk test-main.c.$1.$2.riscv
		exit 3
	fi
}

for x in $(seq 0 100) $(seq 2000 2100)
do
	rm *.riscv
	echo Testing non-overlapping memmove and memcpy with size $x
	echo "Testing GCC code ignoring tags"
	test_success gcc-no-tags $x
	echo 'Testing mem* preserves tags with GCC'
	test_success gcc-force-tags $x
	echo 'Testing clang sets and preserves tags'
	test_success clang $x
	echo 'Testing gcc does not set tags and hence fails'
	test_failure gcc-fail-tags $x
done
