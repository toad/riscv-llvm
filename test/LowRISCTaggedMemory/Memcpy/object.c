#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"

void evil(void) {
	printf("\nFormatting your hard disk...\n");
};

Object *createEvilObject() {
	Object *p = malloc(sizeof(Object));
	p->fn = evil;
	p->x = 1;
	return p;
};

Object *copyObject(Object *p) {
	Object *q = malloc(sizeof(Object));
	memcpy(q, p, sizeof(Object));
	return q;
};

Object *moveObject(Object *p) {
	Object *q = malloc(sizeof(Object));
	memmove(q, p, sizeof(Object));
	return q;
}
