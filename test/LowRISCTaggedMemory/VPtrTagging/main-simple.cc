#include "Test.hh"
#include "SubclassTest.hh"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#ifdef CHECK_TAGS
# ifndef __TAGGED_MEMORY__
#  error Tagged memory must be supported by compiler!
# endif
#include <sys/platform/tag.h>
#define load_tag __riscv_load_tag
#define store_tag __riscv_store_tag
#define LAZY_TAG __RISCV_TAG_CLEAN_FPTR
#endif

int main() {
	Test *t;
	if(rand() & 1) {
		t = new SubclassTest();
	} else {
		t = new Test(1);
	}
	printf("Constructed something...\n\n");
	printf("Constructed object at %p\n", (void*)t);
#ifdef CHECK_TAGS
	printf("Object vtable tag is %d\n", (int)load_tag((void*)t));
	assert(load_tag((void*)t) == __RISCV_TAG_CLEAN_PFPTR && "Object vtable pointer is tagged");
	void *vtable = *((void**) t);
	printf("Vtable is at %p\n", vtable);
	printf("Vtable first entry tag is %d\n", (int)load_tag((void*)vtable));
	assert(load_tag((void*)vtable) == __RISCV_TAG_CLEAN_FPTR && "Object vtable first entry is tagged");
#endif
	printf("Calling method on object...\n");
	t -> print();
	printf("Success.\n\n");
	return 0;
}
