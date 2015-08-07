#!/bin/bash
if ! ./build.sh; then exit 1; fi
if ! spike pk test-main-return.c.clang.riscv | grep "Success"; then echo "TEST FAILED: main-return.c with clang (should succeed)"; exit 2; fi
if ! spike pk test-main-return.c.gcc.riscv | grep "Success"; then echo "TEST FAILED: main-return.c with gcc (should succeed)"; exit 2; fi
if ! spike pk test-main-stack-protection.c.gcc.riscv | grep "Should have terminated by now"; then echo "GCC test didn't crash, maybe stack protector is enabled?"; exit 3; fi
echo Stack protection failed on GCC as expected
if ! spike pk test-main-stack-protection.c.clang.riscv | grep "Illegal store"; then echo "TEST FAILED: main-stack-protection.c succeeded (should fail)"; fi
echo Stack protection succeeded on Clang
echo All tests succeeded

