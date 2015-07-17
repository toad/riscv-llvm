#!/bin/bash
./lowrisc-memmove-test.sh gcc-linux || exit 1
$TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/memmove1.riscv.linux || exit 2
cp memmove1.riscv.linux mnt/bin/
for x in Memcpy MemmoveOverlap MemmoveOverlapForwards
do
	echo Building test $x
	cd $x
	./build-linux-tests.sh || exit 3
	SCRIPTNAME=$(cat build-linux-tests.sh | sed -n "s/^SCRIPT=//p")
	$TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/$SCRIPTNAME $PWD/mnt/ 1048576 1024 || exit 4
	rm -Rf mnt/
	rm $SCRIPTNAME
	cd ..
done
echo "All tests successful"
