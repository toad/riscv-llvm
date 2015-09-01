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
void *px = &x;
int y = 4;
void *py = &y;

void main() {
#ifdef FAKE_TAGS
	store_tag(&px, __RISCV_TAG_CLEAN_POINTER);
	store_tag(&py, __RISCV_TAG_CLEAN_POINTER);
#endif FAKE_TAGS
#ifndef NO_TAGS
	assert(load_tag(&px) == __RISCV_TAG_CLEAN_POINTER);
	assert(load_tag(&py) == __RISCV_TAG_CLEAN_POINTER);
#endif
	printf("*px = %d\n", *(int*)px);
	printf("*py = %d\n", *(int*)py);
	int *tmp = py;
	py = px;
	px = tmp;
#ifdef FAKE_TAGS
	assert(load_tag(&px) == 0);
	assert(load_tag(&py) == 0);
	store_tag(&px, __RISCV_TAG_CLEAN_VOIDPTR);
	store_tag(&py, __RISCV_TAG_CLEAN_VOIDPTR);
#endif
#ifndef NO_TAGS
	assert(load_tag(&px) == __RISCV_TAG_CLEAN_VOIDPTR);
	assert(load_tag(&py) == __RISCV_TAG_CLEAN_VOIDPTR);
#endif
	assert(*(int*)px == 4);
	assert(*(int*)py == 3);
	printf("*px = %d\n", *(int*)px);
	printf("*py = %d\n", *(int*)py);
	printf("Success\n");
}
