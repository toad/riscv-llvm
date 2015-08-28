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

int x = 3;
int *px = &x;
int y = 4;
int *py = &y;

void main() {
#ifdef FAKE_TAGS
	store_tag(&px, __RISCV_TAG_CLEAN_POINTER);
	store_tag(&py, __RISCV_TAG_CLEAN_POINTER);
#endif FAKE_TAGS
#ifndef NO_TAGS
	assert(load_tag(&px) == __RISCV_TAG_CLEAN_POINTER);
	assert(load_tag(&py) == __RISCV_TAG_CLEAN_POINTER);
#endif
	printf("*px = %d\n", *px);
	printf("*py = %d\n", *py);
	int *tmp = py;
	py = px;
	px = tmp;
#ifdef FAKE_TAGS
	assert(load_tag(&px) == 0);
	assert(load_tag(&py) == 0);
	store_tag(&px, __RISCV_TAG_CLEAN_POINTER);
	store_tag(&py, __RISCV_TAG_CLEAN_POINTER);
#endif
#ifndef NO_TAGS
	assert(load_tag(&px) == __RISCV_TAG_CLEAN_POINTER);
	assert(load_tag(&py) == __RISCV_TAG_CLEAN_POINTER);
#endif
	assert(*px == 4);
	assert(*py == 3);
	printf("*px = %d\n", *px);
	printf("*py = %d\n", *py);
	printf("Success\n");
}
