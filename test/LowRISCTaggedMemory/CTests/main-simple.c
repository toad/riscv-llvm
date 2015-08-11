#include <stdio.h>
#include <assert.h>
#ifdef __TAGGED_MEMORY__
#include <sys/platform/tag.h>
#endif

int main(int argc, char **argv) {
#ifdef __lowrisc__
	printf("LowRISC detected at compile time...\n");
#endif
#ifdef __TAGGED_MEMORY__
	printf("Tagged memory detected at compile time...\n");
	/* Very simple test */
	long array[5];
	assert(__riscv_load_tag(&array[0]) == __RISCV_TAG_NONE);
	__riscv_store_tag(&array[0], __RISCV_TAG_READ_ONLY);
	assert(__riscv_load_tag(&array[0]) == __RISCV_TAG_READ_ONLY);
	__riscv_store_tag(&array[0], __RISCV_TAG_NONE);
	assert(__riscv_load_tag(&array[0]) == __RISCV_TAG_NONE);
	array[0] = 0;
	/* Lazy tag gets cleared on a normal write */
	__riscv_store_tag(&array[0], __RISCV_TAG_CLEAN_FPTR);
	array[0] = 1;
	assert(__riscv_load_tag(&array[0]) == __RISCV_TAG_NONE);
	printf("Success!\n");
	return 0;
#else
	printf("Tagged memory not available (at compile time).\n");
	return 1;
#endif
}
