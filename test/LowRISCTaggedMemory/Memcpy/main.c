#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"

int main(int argc, char **argv) {
	Object *p = createEvilObject();
	printf("Calling function on original object...\n");
	p->fn();
	Object *q = copyObject(p);
	printf("Calling function on copy of object...\n");
	q->fn();
	printf("Success!\n");
}
