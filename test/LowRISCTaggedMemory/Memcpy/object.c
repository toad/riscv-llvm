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
	}
}
#endif
