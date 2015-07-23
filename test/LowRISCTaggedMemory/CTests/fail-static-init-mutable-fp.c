#include <stdio.h>
#include <assert.h>

#ifndef NO_TAGS
#include "tag.c"
#endif

void evil() {
	printf("Doing something very evil...\n");
}

void nice() {
	printf("Doing something I'm supposed to do...\n");
}

void (*mutableFN)() = nice;

int main(int argc, char **argv) {
	printf("Calling function\n");
#ifdef FAKE_TAGS
	store_tag(&mutableFN, LAZY_TAG);
#endif
#ifndef NO_TAGS
	assert(load_tag(&mutableFN) == LAZY_TAG);
#endif
	mutableFN();
	printf("Changing function pointer\n");
	void **p = (void**) &mutableFN;
	void *f = (void*) evil;
	*p = f;
#ifndef NO_TAGS
	printf("Tag is now %ld\n", load_tag(&mutableFN));
//	assert(load_tag(&mutableFN) == 0);
#endif
	printf("This should fail...\n");
	mutableFN();
	printf("Called evil function, failure!\n");
	return 1;
}
