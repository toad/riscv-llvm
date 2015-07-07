/* Tests the vptr protection: If a virtual function is called but the vptr
 * is not tagged, we trap.
 *
 * If compiled with GCC, this will run the evil function.
 * If compiled with LLVM with the LowRISC enhancements, it will trap and
 * hence exit silently after "Expect trouble" is printed.
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
	printf("ALL YOUR DATA ARE BELONG TO US!\n\n");
}

void *pEvil = (void*) evil;
void *ppEvil = (void*) (&pEvil);
void *pppEvil = (void*) (&ppEvil);

int main() {
	void *p = pppEvil;
	printf("Casting...\n");
	Test *t = (Test*) p;
	printf("Expect trouble...\n\n");
	t->print();
	return 5;
}
