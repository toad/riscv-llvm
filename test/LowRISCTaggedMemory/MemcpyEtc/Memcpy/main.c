/* Tests copying a struct including function pointers.
 * 
 * Typical operation modes:
 * 
 * Build with GCC: Test basic copying functionality without tags.
 * Build with GCC and FAKE_TAGS: Check that mem* copies tags.
 * Build with Clang: Test that pointer tagging and mem* together copy tags.
 *
 * COMPILE-TIME PARAMETERS:
 *   ARRAY_SIZE: Affects the size of the object. (0 or more)
 *   FAKE_TAGS: If enabled, set the tags manually on the code pointer,
 *     and check them manually when dereferencing it.
 *   NO_TAGS: If enabled, don't check for tags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "object.h"

#ifndef NO_TAGS
# ifndef __TAGGED_MEMORY__
#  error Tagged memory must be supported by compiler!
# endif
#include <sys/platform/tag.h>
#define load_tag __riscv_load_tag
#define store_tag __riscv_store_tag
#define LAZY_TAG __RISCV_TAG_CLEAN_FPTR
#endif

int main(int argc, char **argv) {
	printf("Testing memcpy/memmove with function pointers for array size ");
	printf("%d", ARRAY_SIZE);
#ifdef NO_TAGS
	printf(" (no tags)");
#endif
#ifdef FAKE_TAGS
	printf(" (fake tags)");
#endif
	printf("...\n");
	Object *p = createEvilObject();
	fillArray(p);
	printf("Calling function on original object...\n");
#ifndef NO_TAGS
	printf("Tag on p function pointer is %d\n", (int)load_tag(&(p->fn)));
	assert(load_tag(&(p->fn)) == __RISCV_TAG_CLEAN_FPTR);
#endif
	p->fn();
	Object *q = copyObject(p);
	checkArray(q);
	printf("Calling function on memcpy'ed copy of object...\n");
#ifndef NO_TAGS
	printf("Tag on p function pointer is %d\n", (int)load_tag(&(p->fn)));
	assert(load_tag(&(p->fn)) == __RISCV_TAG_CLEAN_FPTR);
	printf("Tag on q function pointer is %d\n", (int)load_tag(&(q->fn)));
	assert(load_tag(&(q->fn)) == __RISCV_TAG_CLEAN_FPTR);
#endif
	q->fn();
	Object *r = moveObject(p);
	checkArray(r);
	printf("Calling function on memmove'ed copy of object...\n");
#ifndef NO_TAGS
	printf("Tag on p function pointer is %d\n", (int)load_tag(&(p->fn)));
	assert(load_tag(&(p->fn)) == __RISCV_TAG_CLEAN_FPTR);
	printf("Tag on q function pointer is %d\n", (int)load_tag(&(q->fn)));
	assert(load_tag(&(q->fn)) == __RISCV_TAG_CLEAN_FPTR);
	printf("Tag on r function pointer is %d\n", (int)load_tag(&(r->fn)));
	assert(load_tag(&(r->fn)) == __RISCV_TAG_CLEAN_FPTR);
#endif
	r->fn();
	printf("Success!\n");
	deleteObject(p);
	deleteObject(q);
	deleteObject(r);
	return 0;
}
