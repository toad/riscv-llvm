#!/bin/bash
# $1 = compiler type
# $2 = array size
build()
{
	if ! ./build-gcc-linux.sh $1 > /dev/null 2>&1
	then echo Could not build with gcc-linux $1; exit 1; fi
}

rm -Rf mnt
mkdir mnt
mkdir mnt/bin
rm -f *.riscv-linux
rm run-memcpy-tests.sh
echo "#!/bin/ash" > run-memcpy-tests.sh
for x in $(seq 3 100) $(seq 2000 2100)
do
	echo Building $x
	build $x
	mv main.riscv-linux mnt/bin/memmove-overlap-$x
	echo "echo Running test $x" >> run-memcpy-tests.sh
	echo "memmove-overlap-$x || exit" >> run-memcpy-tests.sh
done
echo "echo Completed all tests" >> run-memcpy-tests.sh
chmod +x run-memcpy-tests.sh
