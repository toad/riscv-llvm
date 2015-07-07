/* Tests replacement of an existing vtable.
 *
 * If compiled with GCC, will run the evil function.
 * If compiled with LLVM, will abort because the vptr is read-only.
 */

#include "Test.hh"
#include "SubclassTest.hh"
#include <cstdio>
#include <cstdlib>
#include <cstring>

void evil() {
	printf("Installing rootkit\n");
	printf("Encrypting your hard disk...\n");
	printf("Posting blackmail demand...\n");
}

void *pEvil = (void*) evil;
void *ppEvil = (void*) (&pEvil);

int main() {
	Test *t = new SubclassTest();
	printf("Created a legitimate Test\n");
	t->print();
	printf("Attempting to replace vptr.\n");
	printf("This should fail in non-lazy mode, but succeed in lazy mode.\n");
	memcpy((void*) t, &ppEvil, sizeof(void*));
	printf("Replaced vptr\n");
	printf("Expect trouble!\n");
	printf("In lazy mode, should abort...\n");
	t->print();
	printf("FAILURE: Called invalid pointer?!\n");
	return 5;
}
