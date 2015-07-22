#include "Test.hh"
#include "SubclassTest.hh"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#ifdef CHECK_TAGS
#include "tag.c"
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
	assert(load_tag((void*)t) == LAZY_TAG && "Object vtable pointer is tagged");
	void *vtable = *((void**) t);
	printf("Vtable is at %p\n", vtable);
	assert(load_tag((void*)vtable) == LAZY_TAG && "Object vtable first entry is tagged");
#endif
	printf("Calling method on object...\n");
	t -> print();
	printf("Success.\n\n");
	return 0;
}
