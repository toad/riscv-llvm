#!/bin/bash
if ! ./lowrisc-memmove-test.sh; then echo Failed standard memmove test; exit 1; fi
cd Memcpy
if ! ./build-and-test.sh; then echo Failed memcpy/memmove test; exit 2; fi
cd ..
cd MemmoveOverlap
if ! ./build-and-test.sh; then echo Failed backwards memmove overlapped tags test; exit 3; fi
cd ..
cd MemmoveOverlapForwards
if ! ./build-and-test.sh; then echo Failed forwards memmove overlapped tags test; exit 3; fi
cd ..
cd MemcpyMemmoveRandomTags
if ! ./build-and-test.sh; then echo Failed random tags test; exit 3; fi
cd ..
cd PartiallyAlignedCopy
if ! ./build-and-test.sh; then echo Failed partially aligned copy test; exit 3; fi
echo All tests successful.
