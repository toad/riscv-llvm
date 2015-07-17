#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "object.h"
#include "tag.h"

void evil(void) {
	printf("\nFormatting your hard disk...\n");
};

static Object* allocateSpaceForObject() {
	void *vp = malloc(sizeof(Object)+sizeof(long)*2);
	Object *p = vp + sizeof(long);
#ifndef NO_TAGS
	store_tag(vp, READ_ONLY);
	store_tag(vp + sizeof(Object) + sizeof(long), READ_ONLY);
#endif
	return p;
}

Object *createEvilObject() {
	Object *p = allocateSpaceForObject();
	p->fn = evil;
	p->x = 1;
#ifdef FAKE_TAGS
	store_tag(&(p->fn), LAZY_TAG);
#endif
	return p;
};

Object *copyObject(Object *p) {
	Object *q = allocateSpaceForObject();
	memcpy(q, p, sizeof(Object));
	return q;
};

Object *moveObject(Object *p) {
	Object *q = allocateSpaceForObject();
	memmove(q, p, sizeof(Object));
	return q;
}

void deleteObject(Object *p) {
#ifndef NO_TAGS
	void *vp = (void*) p;
	vp -= sizeof(long);
        store_tag(vp, 0);
        store_tag(vp + sizeof(Object) + sizeof(long), 0);
#endif
	free(vp);
}

#if ARRAY_SIZE > 0
/* Simple checks for memmove basic functionality */
void fillArray(Object *p) {
	int i;
	for(i=0;i<ARRAY_SIZE;i++)
		p->array[i] = i;
}

void checkArray(Object *p) {
	int i;
	for(i=0;i<ARRAY_SIZE;i++) {
		if(p->array[i] != i) {
			printf("ERROR: Array does not contain sequence stored!\n");
			exit(1);
		}
#ifndef NO_TAGS
		assert(load_tag(&(p->array[i])) == 0);
#endif
	}
}
#endif
