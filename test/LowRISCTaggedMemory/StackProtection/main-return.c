/* This tests that the return address gets untagged. If not, a call to another
 * function will fail when it tries to overwrite it. It should be compiled
 * with optimisation turned off i.e. no inlining! */

#include <stdio.h>

void callOne(void);
void callTwo(void);

int main(int argc, char **argv) {
	printf("Testing repeated calls.\n");
	printf("This should complete without error.\n");
	callOne();
	callTwo();
	printf("Success!\n");
	return 0;
}

void callOne(void) {
	printf("Calling first function...\n");
}

void callTwo(void) {
	printf("Calling second function...\n");
}
