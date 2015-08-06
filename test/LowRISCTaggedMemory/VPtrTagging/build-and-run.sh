#!/bin/bash
if ! ./build.sh; then echo Build failed; exit 100; fi
# Unfortunately all of these have different success conditions...
# When new tests are added they will need to be added here.

echo Testing bogus vptr
if ! spike pk test-main-bogus-vptr.cc.gcc.riscv 2>&1 | grep "Misaligned load"; then echo "GCC bogus vptr test should give misaligned load"; exit 1; fi
if spike pk test-main-bogus-vptr.cc.clang.riscv 2>&1 | grep "Misaligned load"; then echo "Clang should prevent misaligned load"; exit 2; fi
# Check for it actually completing too.
if spike pk test-main-bogus-vptr.cc.clang.riscv 2>&1 | grep "FAILURE"; then echo "Clang should prevent misaligned load"; exit 3; fi

echo Testing bogus vtable
if ! spike pk test-main-bogus-vtable.cc.gcc.riscv | grep "Installing rootkit"; then echo "GCC bogus vtable test should run \"malicious\" code"; exit 4; fi
if spike pk test-main-bogus-vtable.cc.clang.riscv | grep "Installing rootkit"; then echo "Clang should prevent \"malicious\" code from running!"; exit 5; fi

echo Testing global initialisation: iostreams hello world
if ! spike pk test-main-iostreams.cc.gcc.riscv | grep "Hello world"; then echo "Static initialisers broken with gcc"; exit 6; fi
if ! spike pk test-main-iostreams.cc.clang.riscv | grep "Hello world"; then echo "Static initialisers broken with Clang"; exit 7; fi

echo Testing vptr overwriting
if ! spike pk test-main-read-only-vptr.cc.gcc.riscv 2>&1 | grep "Misaligned load"; then echo "GCC vptr overwriting test should give misaligned load"; exit 8; fi
if spike pk test-main-read-only-vptr.cc.clang.riscv 2>&1 | grep "Misaligned load"; then echo "Clang vptr overwriting test should prevent misaligned load"; exit 9; fi

echo Testing simple object with tags
if ! spike pk test-main-simple.cc.gcc.riscv | grep "Success"; then echo "GCC failed simple object test"; exit 10; fi
if ! spike pk test-main-simple.cc.clang.riscv | grep "Success"; then echo "Clang failed simple object test"; exit 11; fi

echo Testing malicious vtable replacement
if ! spike pk test-main-vtable-replacement.cc.gcc.riscv | grep "Installing rootkit"; then echo "GCC didn't run \"malicious\" payload as expected in vtable replacement test?"; exit 12; fi 
if spike pk test-main-vtable-replacement.cc.clang.riscv | grep "Installing rootkit"; then echo "Clang didn't prevent \"malicious\" payload from running!"; exit 13; fi

echo Testing static initialisation of tags
if ! spike pk test-main-static.cc.gcc.riscv | grep Success; then echo "Unable to statically initialize objects in GCC"; exit 14; fi
if ! spike pk test-main-static.cc.clang.riscv | grep Success; then echo "Unable to statically initialize objects in Clang"; exit 15; fi

echo All tests successful.
