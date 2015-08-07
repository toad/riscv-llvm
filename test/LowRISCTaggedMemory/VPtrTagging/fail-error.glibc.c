#include <stdio.h>
#include <stdlib.h>

#define eprintf(...) fprintf (stderr, __VA_ARGS__)

void __llvm_riscv_check_tagged_failure() {
	eprintf("FAILURE: Tag check failed\n");
	abort();
}
