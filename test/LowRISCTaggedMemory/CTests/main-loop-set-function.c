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

#define SIZE 48

void nice() {
	printf("Called function...\n");
}

int main(int argc, char **argv) {
	printf("Calling function\n");
	void (*fnarray[SIZE])();
	void (**pfnarray)() = &fnarray;
#ifdef FAKE_TAGS
	store_tag(&pfnarray, __RISCV_TAG_CLEAN_PFPTR);
#endif
#ifndef NO_TAGS
	assert(load_tag(&pfnarray) == __RISCV_TAG_CLEAN_PFPTR);
#endif
	int i;
	for(i=0;i<SIZE;i++) fnarray[i] = nice;
#ifdef FAKE_TAGS
	for(i=0;i<SIZE;i++)
		store_tag(&pfnarray[i], __RISCV_TAG_CLEAN_FPTR);
#endif
#ifndef NO_TAGS
	for(i=0;i<SIZE;i++)
		assert(load_tag(&fnarray[i]) == __RISCV_TAG_CLEAN_FPTR);
#endif
	for(i=0;i<SIZE;i++) {
		fnarray[i]();
	}
	printf("Success!\n");
	return 0;
}
