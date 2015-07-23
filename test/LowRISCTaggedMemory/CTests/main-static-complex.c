#include <stdio.h>
#include <assert.h>

#ifndef NO_TAGS
#include "tag.c"
#endif

void evil() {
	printf("Doing something very evil...\n");
}

void nice1() {
	printf("Doing something I'm supposed to do...\n");
}

void nice2() {
	printf("Doing something else I'm supposed to do...\n");
}

void (*mutableFNs[])() = { nice1, nice2 };

int main(int argc, char **argv) {
#ifdef FAKE_TAGS
	store_tag(&mutableFNs, LAZY_TAG);
	store_tag(&mutableFNs[0], LAZY_TAG);
	store_tag(&mutableFNs[1], LAZY_TAG);
#endif
#ifndef NO_TAGS
	assert(load_tag(&mutableFNs) == LAZY_TAG);
	assert(load_tag(&mutableFNs[0]) == LAZY_TAG);
	assert(load_tag(&mutableFNs[1]) == LAZY_TAG);
#endif
	mutableFNs[0]();
	mutableFNs[1]();
	printf("Swapping mutable functions\n");
	void (*p)() = mutableFNs[0];
	void (*q)() = mutableFNs[1];
	mutableFNs[0] = q;
	mutableFNs[1] = p;
#ifdef FAKE_TAGS
	store_tag(&mutableFNs, LAZY_TAG);
	store_tag(&mutableFNs[0], LAZY_TAG);
	store_tag(&mutableFNs[1], LAZY_TAG);
#endif
#ifndef NO_TAGS
	assert(load_tag(&mutableFNs) == LAZY_TAG);
	assert(load_tag(&mutableFNs[0]) == LAZY_TAG);
	assert(load_tag(&mutableFNs[1]) == LAZY_TAG);
#endif
	mutableFNs[0]();
	mutableFNs[1]();
	printf("Success!\n");
	return 0;
}
