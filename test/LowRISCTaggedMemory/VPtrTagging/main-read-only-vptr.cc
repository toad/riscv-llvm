/* Test for overwriting the vptr.
 *
 * If run with GCC, should crash.
 * If run with LLVM with the LowRISC enhancements, should fail because the
 * vptr is read-only.
 */

#include "Test.hh"
#include "SubclassTest.hh"
#include <cstdio>

int main() {
	int blah = 0; // Originally random
	Test *t;
	if(blah & 1)
		t = new Test(1);
	else
		t = new SubclassTest();
	printf("Constructed something\n");
	t->print();
	printf("Dumped\n"); // Important if problems with tagging vtables.
	// Now test the tagging code...
	printf("Trying to overwrite the vptr...\n");
	printf("This should succeed in lazy mode and fail in non-lazy mode...\n");
	void *p = t;
	long *pl = (long*) p;
	pl[0] = 1;
	printf("Successfully overwritten the vptr!\n");
	printf("Without protection, should crash at this point.\n");
	printf("With protection, should abort...\n");
	t->print();
	printf("ERROR: Called invalid funcion!\n");
	return 5;
}
