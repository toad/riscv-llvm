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

void nice1() {
	printf("Doing something I'm supposed to do...\n");
}

void nice2() {
	printf("Doing something else I'm supposed to do...\n");
}

void (*mutableFNs[])() = { nice1, nice2 };

int main(int argc, char **argv) {
#ifdef FAKE_TAGS
	store_tag(&mutableFNs, __RISCV_TAG_CLEAN_FPTR);
	store_tag(&mutableFNs[0], __RISCV_TAG_CLEAN_FPTR);
	store_tag(&mutableFNs[1], __RISCV_TAG_CLEAN_FPTR);
#endif
#ifndef NO_TAGS
	assert(load_tag(&mutableFNs) == __RISCV_TAG_CLEAN_FPTR);
	assert(load_tag(&mutableFNs[0]) == __RISCV_TAG_CLEAN_FPTR);
	assert(load_tag(&mutableFNs[1]) == __RISCV_TAG_CLEAN_FPTR);
#endif
	mutableFNs[0]();
	mutableFNs[1]();
	printf("Swapping mutable functions\n");
	void (*p)() = mutableFNs[0];
	void (*q)() = mutableFNs[1];
	mutableFNs[0] = q;
	mutableFNs[1] = p;
#ifdef FAKE_TAGS
	store_tag(&mutableFNs, __RISCV_TAG_CLEAN_FPTR);
	store_tag(&mutableFNs[0], __RISCV_TAG_CLEAN_FPTR);
	store_tag(&mutableFNs[1], __RISCV_TAG_CLEAN_FPTR);
#endif
#ifndef NO_TAGS
	assert(load_tag(&mutableFNs) == __RISCV_TAG_CLEAN_FPTR);
	assert(load_tag(&mutableFNs[0]) == __RISCV_TAG_CLEAN_FPTR);
	assert(load_tag(&mutableFNs[1]) == __RISCV_TAG_CLEAN_FPTR);
#endif
	mutableFNs[0]();
	mutableFNs[1]();
	printf("Success!\n");
	return 0;
}
