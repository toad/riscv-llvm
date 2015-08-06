#!/bin/bash
compiler=${1:-gcc-linux}
SCRIPT=run-memcpy-tests.sh
# $1 = compiler type
# $2 = array size
build()
{
	if ! ./build.sh $1 $2 > /dev/null 2>&1
	then echo Could not build $1 $2; exit 1; fi
}

rm -Rf mnt
mkdir mnt
mkdir mnt/bin
rm -f *.riscv-linux
rm -f $SCRIPT
echo "#!/bin/ash" > $SCRIPT
for x in $(seq 0 100) $(seq 2000 2100)
do
	echo Building $x with $compiler
	build $compiler $x
	mv test-main.c.${compiler}.$x.riscv-linux mnt/bin
	echo "echo Running test $x with $compiler" >> $SCRIPT
	echo "test-main.c.${compiler}.$x.riscv-linux || exit" >> $SCRIPT
done
echo "echo Completed all tests on $compiler" >> $SCRIPT
chmod +x run-memcpy-tests.sh
