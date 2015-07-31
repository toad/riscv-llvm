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
#endif

Test globalT1(1);
Test globalT2(2);
Test *globalPT = &globalT1;
SubclassTest globalS1;
Test *globalPS = &globalS1;

// Array of objects
Test arr[] = { Test(1), Test(2), Test(3) };
// Array of pointers
Test *parr[] = { &globalT1, &globalT2, &globalS1 };
// Pointer to pointer
Test **pparr = &parr[0];
Test ***ppparr = &pparr;

int main() {
	printf("Starting C++ global init test\n");
#ifdef CHECK_TAGS
	assert(load_tag(&globalT1) == __RISCV_TAG_CLEAN_PFPTR); // vptr
	assert(load_tag(&globalT2) == __RISCV_TAG_CLEAN_PFPTR); // vptr
	assert(load_tag(&globalPT) == __RISCV_TAG_CLEAN_SENSITIVE);
	assert(load_tag(&globalS1) == __RISCV_TAG_CLEAN_PFPTR);
	assert(load_tag(&globalPS) == __RISCV_TAG_CLEAN_SENSITIVE);
	assert(load_tag(&parr[0] == __RISCV_TAG_CLEAN_SENSITIVE);
	assert(load_tag(&parr[1] == __RISCV_TAG_CLEAN_SENSITIVE);
	assert(load_tag(&parr[2] == __RISCV_TAG_CLEAN_SENSITIVE);
	assert(load_tag(&arr[0] == __RISCV_TAG_CLEAN_PFPTR);
	assert(load_tag(&arr[1] == __RISCV_TAG_CLEAN_PFPTR);
	assert(load_tag(&arr[2] == __RISCV_TAG_CLEAN_PFPTR);
	assert(load_tag(&pparr) == __RISCV_TAG_CLEAN_SENSITIVE);
	assert(load_tag(&ppparr) == __RISCV_TAG_CLEAN_SENSITIVE);
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
