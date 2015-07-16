#!/bin/bash
# $1 = compiler type
# $2 = array size
build()
{
	if ! ./build.sh gcc-linux $1 > /dev/null 2>&1
	then echo Could not build with gcc-linux $1; exit 1; fi
}

rm -Rf mnt
mkdir mnt
mkdir mnt/bin
rm -f *.riscv-linux
rm run-memcpy-tests.sh
echo "#!/bin/ash" > run-memcpy-tests.sh
for x in $(seq 0 100) $(seq 2000 2100)
do
	echo Building $x
	build $x
	mv test-main.c.gcc-linux.$x.riscv-linux mnt/bin
	echo "echo Running test $x" >> run-memcpy-tests.sh
	echo "test-main.c.gcc-linux.$x.riscv-linux || exit" >> run-memcpy-tests.sh
done
echo "echo Completed all tests" >> run-memcpy-tests.sh
chmod +x run-memcpy-tests.sh
