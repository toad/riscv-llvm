#!/bin/bash
if ! ./build.sh; then echo Build failed; exit 100; fi
# Unfortunately all of these have different success conditions...
# When new tests are added they will need to be added here.

echo Testing bogus vptr
if ! spike pk test-main-bogus-vptr.cc.gcc.riscv 2>&1 | grep "User load segfault"; then echo "GCC bogus vptr test should give \"User load segfault\""; exit 1; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-bogus-vptr.cc.gcc-linux.riscv | grep "Segmentation fault"; then echo "GCC-linux bogus vptr test should give segfault"; exit 1; fi
if $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-bogus-vptr.cc.gcc-linux.riscv | grep "FAILURE: Called invalid pointer"; then echo "GCC-linux bogus vptr test should give segfault but not return"; exit 1; fi
if ! spike pk test-main-bogus-vptr.cc.clang.riscv 2>&1 | grep "FAILURE: Tag check failed"; then echo "Clang should prevent bogus call"; exit 2; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-bogus-vptr.cc.clang-linux.riscv | grep "FAILURE: Tag check failed"; then echo "Clang-Linux bogus vptr test should give tag check failed"; exit 1; fi

echo Testing bogus vtable
if ! spike pk test-main-bogus-vtable.cc.gcc.riscv | grep "Installing rootkit"; then echo "GCC bogus vtable test should run \"malicious\" code"; exit 4; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-bogus-vtable.cc.gcc-linux.riscv | grep "Installing rootkit"; then echo "GCC linux bogus vtable test should run \"malicious\" code"; exit 4; fi
if spike pk test-main-bogus-vtable.cc.clang.riscv | grep "Installing rootkit"; then echo "Clang should prevent \"malicious\" code from running!"; exit 5; fi
if ! spike pk test-main-bogus-vtable.cc.clang.riscv | grep "FAILURE: Tag check failed"; then echo "Clang should prevent \"malicious\" code from running!"; exit 5; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-bogus-vtable.cc.clang-linux.riscv | grep "FAILURE: Tag check failed"; then echo "Clang should prevent \"malicious\" code from running!"; exit 5; fi

echo Testing global initialisation: iostreams hello world
if ! spike pk test-main-iostreams.cc.gcc.riscv | grep "Hello world"; then echo "Static initialisers broken with gcc"; exit 6; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-iostreams.cc.gcc-linux.riscv | grep "Hello world"; then echo "Static initialisers broken with gcc-linux"; exit 6; fi
if ! spike pk test-main-iostreams.cc.clang.riscv | grep "Hello world"; then echo "Static initialisers broken with Clang"; exit 7; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-iostreams.cc.clang-linux.riscv | grep "Hello world"; then echo "Static initialisers broken with clang-linux"; exit 6; fi

echo Testing vptr overwriting
if ! spike pk test-main-read-only-vptr.cc.gcc.riscv 2>&1 | grep "User load segfault"; then echo "GCC vptr overwriting test should give \"User load segfault\""; exit 8; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-read-only-vptr.cc.gcc-linux.riscv | grep "Segmentation fault"; then echo "GCC-linux vptr overwriting test should give segfault"; exit 8; fi
if spike pk test-main-read-only-vptr.cc.clang.riscv 2>&1 | grep "Segmentation fault"; then echo "Clang vptr overwriting test should prevent misaligned load"; exit 9; fi
if ! spike pk test-main-read-only-vptr.cc.clang.riscv 2>&1 | grep "FAILURE: Tag check failed"; then echo "Clang vptr overwriting test should prevent misaligned load"; exit 9; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-read-only-vptr.cc.clang-linux.riscv | grep "FAILURE: Tag check failed"; then echo "Clang-Linux vptr overwriting test should prevent misaligned load"; exit 9; fi

echo Testing simple object with tags
if ! spike pk test-main-simple.cc.gcc.riscv | grep "Success"; then echo "GCC failed simple object test"; exit 10; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-simple.cc.gcc-linux.riscv | grep "Success"; then echo "GCC-linux failed simple object test"; exit 10; fi
if ! spike pk test-main-simple.cc.clang.riscv | grep "Success"; then echo "Clang failed simple object test"; exit 11; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-simple.cc.clang-linux.riscv | grep "Success"; then echo "Clang-linux failed simple object test"; exit 10; fi

echo Testing malicious vtable replacement
if ! spike pk test-main-vtable-replacement.cc.gcc.riscv | grep "Installing rootkit"; then echo "GCC didn't run \"malicious\" payload as expected in vtable replacement test?"; exit 12; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-vtable-replacement.cc.gcc-linux.riscv | grep "Installing rootkit"; then echo "GCC-linux didn't run \"malicious\" payload as expected in vtable replacement test?"; exit 12; fi
if spike pk test-main-vtable-replacement.cc.clang.riscv | grep "Installing rootkit"; then echo "Clang didn't prevent \"malicious\" payload from running!"; exit 13; fi
if ! spike pk test-main-vtable-replacement.cc.clang.riscv | grep "FAILURE: Tag check failed"; then echo "Clang didn't prevent \"malicious\" payload from running!"; exit 13; fi
if spike pk test-main-vtable-replacement.cc.clang.riscv | grep "Installing rootkit"; then echo "Clang didn't prevent \"malicious\" payload from running!"; exit 13; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-vtable-replacement.cc.clang-linux.riscv | grep "FAILURE: Tag check failed"; then echo "Clang-Linux didn't prevent \"malicious\" payload from running!"; exit 13; fi
if $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-vtable-replacement.cc.clang-linux.riscv | grep "Installing rootkit"; then echo "Clang-Linux didn't prevent \"malicious\" payload from running!"; exit 13; fi

echo Testing static initialisation of tags
if ! spike pk test-main-static.cc.gcc.riscv | grep Success; then echo "Unable to statically initialize objects in GCC"; exit 14; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-static.cc.gcc-linux.riscv | grep Success; then echo "Unable to statically initialize objects in GCC-linux"; exit 14; fi
if ! spike pk test-main-static.cc.clang.riscv | grep Success; then echo "Unable to statically initialize objects in Clang"; exit 15; fi
if ! $TOP/lowrisc-chip/riscv-tools/run_test_linux_spike.sh $PWD/test-main-static.cc.clang-linux.riscv | grep Success; then echo "Unable to statically initialize objects in Clang-linux"; exit 14; fi

echo All tests successful.
