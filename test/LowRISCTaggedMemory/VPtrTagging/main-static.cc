#include "Test.hh"
#include "SubclassTest.hh"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#ifdef CHECK_TAGS
# ifndef __TAGGED_MEMORY__
#  error Tagged memory must be supported by compiler!
# endif
#include <sys/platform/tag.h>
#define load_tag __riscv_load_tag
#define store_tag __riscv_store_tag
#define LAZY_TAG __RISCV_TAG_LAZY
#endif

Test globalT1(1);
Test globalT2(2);
Test *globalPT = &globalT1;
SubclassTest globalS1;
Test *globalPS = &globalS1;

Test arr[] = { Test(1), Test(2), Test(3) };
Test *parr[] = { &globalT1, &globalT2, &globalS1 };

int main() {
	printf("Starting C++ global init test\n");
#ifdef CHECK_TAGS
	assert(load_tag(&globalT1) == LAZY_TAG); // vptr
	assert(load_tag(&globalT2) == LAZY_TAG); // vptr
	assert(load_tag(&globalPT) == LAZY_TAG);
	assert(load_tag(&globalS1) == LAZY_TAG);
	assert(load_tag(&globalPS) == LAZY_TAG);
	assert(load_tag(&parr[0] == LAZY_TAG);
	assert(load_tag(&parr[1] == LAZY_TAG);
	assert(load_tag(&parr[2] == LAZY_TAG);
	assert(load_tag(&arr[0] == LAZY_TAG);
	assert(load_tag(&arr[1] == LAZY_TAG);
	assert(load_tag(&arr[2] == LAZY_TAG);
#endif // CHECK_TAGS
	printf("Invoking C++ objects...\n");
	globalT1.print();
	globalT2.print();
	globalPT->print();
	globalS1.print();
	globalPS->print();
	for(int i=0;i<2;i++)
		arr[i].print();
	for(int i=0;i<2;i++)
		parr[i]->print();
	printf("Success.\n\n");
	return 0;
}
