#include "Test.hh"
#include "SubclassTest.hh"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main() {
	void *p = malloc(16);
	const char* test = "This is a string";
	memcpy(p, test, 16);
	printf("Cast to void\n");
	Test *t = static_cast<Test*> (p);
	printf("Should fail with trap, NOT segfault...");
	t->print();
	printf("FAILURE: Called invalid pointer?!\n");
	return 5;
}
