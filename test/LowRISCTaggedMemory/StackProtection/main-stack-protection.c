/* Test for LowRISC stack protection. Should be built with
 * -fno-stack-protector (as we don't use a cookie, we tag the spilled
 * registers instead).
 */

#include <stdio.h>

void test(void);

int main() {
	test();
	printf("Successfully terminated, WTF?\n");
	return 1;
}

void test() {
	printf("Should fail to write on writing element 16...\n");
	int arr[16];
	int i=0;
	for(i=0;i<64;i++) {
		printf("Filling array element %d of %d\n", i, 16);
		arr[i] = i;
	}
	printf("Should have terminated by now! Trying to return to bogus address...\n");
}
