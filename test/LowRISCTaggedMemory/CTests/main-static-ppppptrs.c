#include <stdio.h>
#include <assert.h>

#ifndef NO_TAGS
# ifndef __TAGGED_MEMORY__
#  error Tagged memory must be supported by compiler!
# endif
#include <sys/platform/tag.h>
#define load_tag __riscv_load_tag
#define store_tag __riscv_store_tag
#define LAZY_TAG __RISCV_TAG_LAZY
#endif

void nice() {
	printf("Doing something I'm supposed to do...\n");
}

void (*pfn)() = nice;
void (**ppfn)() = &pfn;
void (***pppfn)() = &ppfn;
void (****ppppfn)() = &pppfn;

int main(int argc, char **argv) {
#ifdef FAKE_TAGS
	store_tag(&pfn, LAZY_TAG);
	store_tag(&ppfn, LAZY_TAG);
	store_tag(&pppfn, LAZY_TAG);
	store_tag(&ppppfn, LAZY_TAG);
#endif
#ifndef NO_TAGS
	assert(load_tag(&pfn) == LAZY_TAG);
	assert(load_tag(&ppfn) == LAZY_TAG);
	assert(load_tag(&pppfn) == LAZY_TAG);
	assert(load_tag(&ppppfn) == LAZY_TAG);
#endif
	pfn();
	(*ppfn)();
	(**ppfn)();
	(***pppfn)();
	(****ppppfn)();
	printf("Success!\n");
	return 0;
}
