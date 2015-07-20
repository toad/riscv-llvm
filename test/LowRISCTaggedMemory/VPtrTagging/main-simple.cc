#include "Test.hh"
#include "SubclassTest.hh"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main() {
	Test *t;
	if(rand() & 1) {
		t = new SubclassTest();
	} else {
		t = new Test(1);
	}
	t -> print();
	return 0;
}
