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
	mutableFN = evil;
#ifdef FAKE_TAGS
	store_tag(&mutableFN, LAZY_TAG);
#endif
#ifndef NO_TAGS
	assert(load_tag(&mutableFN) == LAZY_TAG);
#endif
	printf("Calling new function\n");
	mutableFN();
	printf("Success!\n");
	return 0;
}
