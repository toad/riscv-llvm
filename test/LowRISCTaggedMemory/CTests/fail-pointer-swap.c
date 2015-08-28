#include <stdio.h>
#include <assert.h>

int lastVal = 0;

void fn1() {
	printf("Called fn1\n");
	lastVal = 1;
}

void fn2() {
	printf("Called fn2\n");
	lastVal = 2;
}

long val;
long *pval = &val;
long val2;
long *pval2 = &val2;

void (*pf1)() = fn1;
void (*pf2)() = fn2;
void (**ppf1)() = &pf1;
void (**ppf2)() = &pf2;

void swap(void** pp1, void** pp2) {
	void* p1 = *pp1;
	void *p2 = *pp2;
	*pp1 = p2;
	*pp2 = p1;
}

void main() {
	val = 1;
	val2 = 2;
	printf("First pointer gives %d\n", *pval);
	printf("Second pointer gives %d\n", *pval2);
	swap(&pval, &pval2);
	assert(*pval == 2);
	assert(*pval2 == 1);
	printf("First pointer gives %d\n", *pval);
	printf("Second pointer gives %d\n", *pval2);
	swap(&ppf1, &ppf2);
	(*ppf1)();
	assert(lastVal == 2);
	(*ppf2)();
	assert(lastVal == 1);
	printf("Success\n");
}
