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
	memcpy((void*) t, &ppEvil, sizeof(void*));
	printf("Replaced vptr\n");
	printf("Expect trouble!\n");
	t->print();
	printf("FAILURE: Called invalid pointer?!\n");
	return 5;
}
