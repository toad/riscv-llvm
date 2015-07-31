#!/bin/bash
if ! ./build.sh; then exit 1; fi
if ! spike pk test-main-return.c.clang.riscv | grep "Success" > /dev/null 2>&1; then echo "TEST FAILED: main-return.c (should succeed)"; exit 2; fi
if spike pk test-main-stack-protection.c.clang.riscv | grep "Success" > /dev/null 2>&1; then echo "TEST FAILED: main-stack-protection.c succeeded (should fail)"; fi
echo All tests succeeded

