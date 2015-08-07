#include <stdio.h>
#include <stdlib.h>

void __llvm_riscv_check_tagged_failure() {
	printf("FAILURE: Tag check failed\n");
	abort();
}
