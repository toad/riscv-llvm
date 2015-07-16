/* Tests copying a struct including function pointers.
 * COMPILE-TIME PARAMETERS:
 *   ARRAY_SIZE: Affects the size of the object. (0 or more)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"

int main(int argc, char **argv) {
	Object *p = createEvilObject();
	fillArray(p);
	printf("Calling function on original object...\n");
	p->fn();
	Object *q = copyObject(p);
	checkArray(q);
	printf("Calling function on memcpy'ed copy of object...\n");
	q->fn();
	Object *r = moveObject(p);
	checkArray(r);
	printf("Calling function on memmove'ed copy of object...\n");
	r->fn();
	printf("Success!\n");
}
