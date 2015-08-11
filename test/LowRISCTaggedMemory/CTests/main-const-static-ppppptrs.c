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

void nice() {
	printf("Doing something I'm supposed to do...\n");
}

const void (* const pfn)() = nice;
const void (** const ppfn)() = &pfn;
const void (*** const pppfn)() = &ppfn;
const void (**** const ppppfn)() = &pppfn;

int main(int argc, char **argv) {
	printf("Function is at %p\n", nice);
#ifdef FAKE_TAGS
	store_tag(&pfn, __RISCV_TAG_CLEAN_FPTR);
	store_tag(&ppfn, __RISCV_TAG_CLEAN_PFPTR);
	store_tag(&pppfn, __RISCV_TAG_CLEAN_PFPTR);
	store_tag(&ppppfn, __RISCV_TAG_CLEAN_PFPTR);
#endif
#ifndef NO_TAGS
	printf("tag on pfn = %d\n", (int)load_tag(&pfn));
	printf("tag on ppfn = %d\n", (int)load_tag(&ppfn));
	printf("tag on pppfn = %d\n", (int)load_tag(&pppfn));
	printf("tag on ppppfn = %d\n", (int)load_tag(&ppppfn));
	assert(load_tag(&pfn) == __RISCV_TAG_CLEAN_FPTR);
	assert(load_tag(&ppfn) == __RISCV_TAG_CLEAN_PFPTR);
	assert(load_tag(&pppfn) == __RISCV_TAG_CLEAN_PFPTR);
	assert(load_tag(&ppppfn) == __RISCV_TAG_CLEAN_PFPTR);
#endif
	printf("Calling function\n");
	pfn();
	(*ppfn)();
	(**ppfn)();
	(***pppfn)();
	(****ppppfn)();
	printf("Success!\n");
	return 0;
}
