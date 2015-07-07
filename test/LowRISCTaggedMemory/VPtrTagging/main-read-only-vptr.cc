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
	t->print();
	// Now test the tagging code...
	printf("Trying to overwrite the vptr...\n");
	printf("This should fail!\n");
	void *p = t;
	long *pl = (long*) p;
	pl[0] = 1;
	printf("OVERWRITTEN THE VPTR. THIS IS BAD! Crashing...\n");
	t->print();
	return 5;
}
