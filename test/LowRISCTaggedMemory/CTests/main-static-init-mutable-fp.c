#include <stdio.h>
#include <assert.h>

#ifndef NO_TAGS
# ifndef __TAGGED_MEMORY__
#  error Tagged memory must be supported by compiler!
# endif
#include <sys/platform/tag.h>
#define load_tag __riscv_load_tag
#define store_tag __riscv_store_tag
#define LAZY_TAG __RISCV_TAG_CLEAN_FPTR
#endif

void evil() {
	printf("Doing something very evil...\n");
}

void nice() {
	printf("Doing something I'm supposed to do...\n");
}

void (*mutableFN)() = nice;

int main(int argc, char **argv) {
	printf("Calling function\n");
#ifdef FAKE_TAGS
	store_tag(&mutableFN, __RISCV_TAG_CLEAN_FPTR);
#endif
#ifndef NO_TAGS
	assert(load_tag(&mutableFN) == __RISCV_TAG_CLEAN_FPTR);
#endif
	mutableFN();
	printf("Changing function pointer\n");
	mutableFN = evil;
#ifdef FAKE_TAGS
	store_tag(&mutableFN, __RISCV_TAG_CLEAN_FPTR);
#endif
#ifndef NO_TAGS
	assert(load_tag(&mutableFN) == __RISCV_TAG_CLEAN_FPTR);
#endif
	printf("Calling new function\n");
	mutableFN();
	printf("Success!\n");
	return 0;
}
