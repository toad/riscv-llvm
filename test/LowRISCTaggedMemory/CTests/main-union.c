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
	printf("Called function...\n");
}

typedef union {
	void (*pfn)();
	long value;
} fnptr_long_union;

int main(int argc, char **argv) {
	// Volatile to avoid it squashing the access to u.value.
	volatile fnptr_long_union u;
#ifndef NO_TAGS
	assert(load_tag(&u) == 0);
#endif
	u.pfn = nice;
#ifdef FAKE_TAGS
	store_tag(&u, __RISCV_TAG_CLEAN_FPTR);
#endif
#ifndef NO_TAGS
	assert(load_tag(&u) == __RISCV_TAG_CLEAN_FPTR);
#endif
	u.pfn();
	u.value = 42;
#ifndef NO_TAGS
	assert(load_tag(&u) == 0);
#endif
	assert(u.value == 42);
	printf("Success!\n");
	return 0;
}
