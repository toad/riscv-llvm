#!/bin/bash
cd CTests
./build-and-run.sh || exit 1
echo "C tagging tests successful"
cd ..
cd MemcpyEtc
./run-tests.sh || exit 2
cd ..
echo "Memory tests successful"
cd StackProtection
./build-and-run.sh || exit 3
cd ..
echo "Stack protection tests successful"
cd VPtrTagging
./build-and-run.sh || exit 4
echo "C++ virtual pointer tagging tests successful"
echo "Completed all tests!"

