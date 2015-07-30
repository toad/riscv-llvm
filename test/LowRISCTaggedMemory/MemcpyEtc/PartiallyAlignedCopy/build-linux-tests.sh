#!/bin/bash
# $1 = compiler type
# $2 = array size
SCRIPT=run-memmove-partially-aligned-tests.sh
build()
{
	if ! ./build-gcc-linux.sh $1 > /dev/null 2>&1
	then echo Could not build with gcc-linux $1; exit 1; fi
}

rm -Rf mnt
mkdir mnt
mkdir mnt/bin
rm -f *.riscv-linux
rm -f $SCRIPT
echo "#!/bin/ash" > $SCRIPT
for x in $(seq 0 100) $(seq 2000 2100)
do
	echo Building $x
	build $x
	mv main.riscv-linux mnt/bin/memmove-partially-aligned-$x
	echo "echo Running test $x" >> $SCRIPT
	echo "memmove-partially-aligned-$x || exit" >> $SCRIPT
done
echo "echo Completed all tests" >> $SCRIPT
chmod +x $SCRIPT
