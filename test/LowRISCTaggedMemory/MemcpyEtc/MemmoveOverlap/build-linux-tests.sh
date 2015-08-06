#!/bin/bash
compiler=${1:-gcc-linux}
# $1 = compiler type
# $2 = array size
SCRIPT=run-memmove-overlap-tests.sh
build()
{
	if ! ./build-${compiler}.sh $1 > /dev/null 2>&1
	then echo Could not build with gcc-linux $1; exit 1; fi
}

rm -Rf mnt
mkdir mnt
mkdir mnt/bin
rm -f *.riscv-linux
rm -f $SCRIPT
echo "#!/bin/ash" > $SCRIPT
for x in $(seq 3 100) $(seq 2000 2100)
do
	echo Building $x with $compiler
	build $x
	mv main.${compiler}.riscv-linux mnt/bin/memmove-overlap-${compiler}-$x
	echo "echo Running test $x for $compiler" >> $SCRIPT
	echo "memmove-overlap-${compiler}-$x || exit" >> $SCRIPT
done
echo "echo Completed all tests" >> $SCRIPT
chmod +x $SCRIPT
