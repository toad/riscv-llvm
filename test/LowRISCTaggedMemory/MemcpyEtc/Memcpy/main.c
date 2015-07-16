/* Tests copying a struct including function pointers.
 * 
 * Typical operation modes:
 * 
 * Build with GCC: Test basic copying functionality without tags.
 * Build with GCC and FAKE_TAGS: Check that mem* copies tags.
 * Build with Clang: Test that pointer tagging and mem* together copy tags.
 *
 * COMPILE-TIME PARAMETERS:
 *   ARRAY_SIZE: Affects the size of the object. (0 or more)
 *   FAKE_TAGS: If enabled, set the tags manually on the code pointer,
 *     and check them manually when dereferencing it.
 *   NO_TAGS: If enabled, don't check for tags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "object.h"
#include "tag.h"

int main(int argc, char **argv) {
	printf("Testing memcpy/memmove with function pointers for ARRAY_SIZE size...\n");
	Object *p = createEvilObject();
	fillArray(p);
	printf("Calling function on original object...\n");
#ifndef NO_TAGS
	assert(load_tag(&(p->fn)) == LAZY_TAG);
#endif
	p->fn();
	Object *q = copyObject(p);
	checkArray(q);
	printf("Calling function on memcpy'ed copy of object...\n");
#ifndef NO_TAGS
	assert(load_tag(&(p->fn)) == LAZY_TAG);
#endif
	q->fn();
	Object *r = moveObject(p);
	checkArray(r);
	printf("Calling function on memmove'ed copy of object...\n");
#ifndef NO_TAGS
	assert(load_tag(&(p->fn)) == LAZY_TAG);
#endif
	r->fn();
	printf("Success!\n");
	return 0;
}
