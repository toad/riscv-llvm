/* This generates warnings and violates aliasing rules. It fails with
 * code pointer tagging unless EXPENSIVE_VOID_COMPAT_HACK is enabled. */

#include <stdio.h>
#include <assert.h>

#ifndef NO_TAGS
# ifndef __TAGGED_MEMORY__
#  error Tagged memory must be supported by compiler!
# endif
#include <sys/platform/tag.h>
#define load_tag __riscv_load_tag
#define store_tag __riscv_store_tag
#endif

int lastVal = 0;

void fn1() {
	printf("Called fn1\n");
	lastVal = 1;
}

void fn2() {
	printf("Called fn2\n");
	lastVal = 2;
}

long val;
long *pval = &val;
long val2;
long *pval2 = &val2;

void (*pf1)() = fn1;
void (*pf2)() = fn2;
void (**ppf1)() = &pf1;
void (**ppf2)() = &pf2;

void swap(void** pp1, void** pp2) {
        printf("Swap\n");
	void *p1 = *pp1;
	void *p2 = *pp2;
	*pp1 = p2;
	*pp2 = p1;
}

void main() {
	val = 1;
	val2 = 2;
	printf("First pointer gives %d\n", *pval);
	printf("Second pointer gives %d\n", *pval2);
#ifndef NO_TAGS
        printf("Tag on first pointer is %d\n", (int) load_tag(&pval));
        printf("Tag on second pointer is %d\n", (int) load_tag(&pval2));
        assert(load_tag(&pval) == __RISCV_TAG_CLEAN_POINTER);
        assert(load_tag(&pval2) == __RISCV_TAG_CLEAN_POINTER);
#endif
	swap(&pval, &pval2);
	assert(*pval == 2);
	assert(*pval2 == 1);
	printf("First pointer gives %d\n", *pval);
	printf("Second pointer gives %d\n", *pval2);
#ifndef NO_TAGS
        printf("Tag on first pointer is %d\n", (int) load_tag(&pval));
        printf("Tag on second pointer is %d\n", (int) load_tag(&pval2));
        assert(load_tag(&pval) == __RISCV_TAG_CLEAN_VOIDPTR);
        assert(load_tag(&pval2) == __RISCV_TAG_CLEAN_VOIDPTR);
        printf("Tag on first indirect function pointer is %d\n", (int) load_tag(&ppf1));
        printf("Tag on second indirect function pointer is %d\n", (int) load_tag(&ppf2));
        assert(load_tag(&ppf1) == __RISCV_TAG_CLEAN_PFPTR);
        assert(load_tag(&ppf2) == __RISCV_TAG_CLEAN_PFPTR);
#endif
	swap(&ppf1, &ppf2);
#ifndef NO_TAGS
        assert(load_tag(&ppf1) == __RISCV_TAG_CLEAN_VOIDPTR);
        assert(load_tag(&ppf2) == __RISCV_TAG_CLEAN_VOIDPTR);
        printf("Tag on first indirect function pointer is %d\n", (int) load_tag(&ppf1));
        printf("Tag on second indirect function pointer is %d\n", (int) load_tag(&ppf2));
#endif
	(*ppf1)();
	assert(lastVal == 2);
	(*ppf2)();
	assert(lastVal == 1);
	printf("Success\n");
}
