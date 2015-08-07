#!/bin/bash
cd CTests
./build-and-run.sh || exit 1
cd ..
cd StackProtection
./build-and-run.sh || exit 2
cd ..
cd VPtrTagging
./build-and-run.sh || exit 3
cd ..
cd MemcpyEtc
./run-all-linux-tests.sh || exit 4
echo All linux tests successful
echo 'mem* newlib/pk tests not run'
