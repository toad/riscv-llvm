#!/bin/bash
if ! ./lowrisc-memmove-test.sh; then echo Failed standard memmove test; exit 1; fi
cd Memcpy
if ! ./build-and-test.sh; then echo Failed memcpy/memmove test; exit 2; fi
cd ..
cd MemmoveOverlap
if ! ./build-and-test.sh; then echo Failed memmove overlapped tags test; exit 3; fi
echo All tests successful.
