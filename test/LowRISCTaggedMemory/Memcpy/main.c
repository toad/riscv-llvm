#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"

int main(int argc, char **argv) {
	Object *p = createEvilObject();
	printf("Calling function on original object...\n");
	p->fn();
	Object *q = copyObject(p);
	printf("Calling function on memcpy'ed copy of object...\n");
	q->fn();
	Object *r = moveObject(p);
	printf("Calling function on memmove'ed copy of object...\n");
	r->fn();
	printf("Success!\n");
}
