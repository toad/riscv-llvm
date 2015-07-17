#!/bin/bash
SCRIPT=run-mem-tests.sh
rm $SCRIPT
echo "#!/bin/ash" > $SCRIPT
rm -Rf mnt
mkdir mnt
mkdir mnt/bin
./lowrisc-memmove-test.sh gcc-linux || exit 1
cp memmove1.riscv.linux mnt/bin/
echo "memmove1.riscv.linux || exit 1" >> $SCRIPT
for x in Memcpy MemmoveOverlap MemmoveOverlapForwards
do
	echo Building test $x
	cd $x
	./build-linux-tests.sh || exit 2
	cd ..
	mv $x/mnt/bin/* mnt/bin/ || exit 3
	rmdir $x/mnt/bin || exit 3
	rmdir $x/mnt || exit 3
done
mv Memcpy/run-memcpy-tests.sh MemmoveOverlap/run-memmove-overlap-tests.sh MemmoveOverlapForwards/run-memmove-overlap-forward-tests.sh mnt/bin/ || exit 4
echo "run-memcpy-tests.sh || exit 1" >> $SCRIPT
echo "run-memmove-overlap-tests.sh || exit 2" >> $SCRIPT
echo "run-memmove-overlap-forward-tests.sh || exit 3" >> $SCRIPT
echo "All tests successful" >> $SCRIPT
WD=$PWD
cd $TOP/lowrisc-chip/riscv-tools
echo Creating root image...
./make_root.sh $WD/$SCRIPT $WD/mnt/ 4194304 4096 || exit 5
echo "Booting Spike to run tests..."
echo "Note that this will not exit when finished, look for \"All tests successful\""
spike +disk=root.bin linux-3.14.13/vmlinux
rm -Rf root.bin $WD/mnt/
