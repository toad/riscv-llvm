#!/bin/bash
mkdir -p mnt/bin
echo Testing memmove built with GCC on linux
./lowrisc-memmove-test.sh gcc-linux || exit 1
$TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/memmove1.riscv.linux || exit 2
rm memmove1.riscv.linux
echo Testing memmove built with Clang on linux
./lowrisc-memmove-test.sh clang-linux || exit 1
$TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/memmove1.riscv.linux || exit 2
for x in Memcpy MemmoveOverlap MemmoveOverlapForwards MemcpyMemmoveRandomTags PartiallyAlignedCopy
do
	for compiler in gcc-linux clang-linux
	do
		echo Building test $x
		cd $x
		./build-linux-tests.sh $compiler || exit 3
		SCRIPTNAME=$(cat build-linux-tests.sh | sed -n "s/^SCRIPT=//p")
		$TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/$SCRIPTNAME $PWD/mnt/ 1048576 1024 || exit 4
		rm -Rf mnt/
		rm $SCRIPTNAME
		cd ..
	done
done
echo "All tests successful"
