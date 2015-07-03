#include <cstdio>
#include "SubclassTest.hh"

SubclassTest::SubclassTest() : Test(0) {
	printf("Constructing a SubclassTest\n");
	x = 3;
}

void SubclassTest::print() {
	printf("Subclass test\n");
}
