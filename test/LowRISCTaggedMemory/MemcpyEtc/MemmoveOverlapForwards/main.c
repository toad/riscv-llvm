#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tag.h"

#define LAZY 4
#ifndef LENGTH
#define LENGTH 3
#else
# if LENGTH < 3
#  error LENGTH must be at least 3
# endif
#endif

int main() {
	printf("Starting test...\n");
	printf("Running forwards memmove test with length %d\n\n", LENGTH);
	long arr[LENGTH];
	int i=0;
	for(i=0;i<LENGTH;i++) arr[i] = i;
	store_tag(&arr[1], LAZY);
	store_tag(&arr[2], LAZY);
	printf("Memcopy from %p to %p length %ld\n\n", &arr[1], &arr[0], sizeof(long)*(LENGTH-1));
	memmove(&arr[0], &arr[1], sizeof(long)*(LENGTH-1));
	if(load_tag(&arr[0]) != LAZY) {
		printf("ERROR: Tag on [1] should be LAZY=%d, is %d\n", LAZY, load_tag(&arr[1]));
		exit(2);
	}
	if(load_tag(&arr[1]) != LAZY) {
		printf("ERROR: Tag on [2] should be LAZY=%d, is %d\n", LAZY, load_tag(&arr[2]));
		exit(3);
	}
	for(i=0;i<LENGTH-1;i++) {
		if(arr[i] !=i+1) {
			printf("Array[%d]=%ld should be %d\n", i, arr[i], i+1);
			printf("Dump:\n\n");
			for(i=0;i<LENGTH;i++) {
				printf("Array[%d] = %ld\n", i, arr[i]);
			}
			exit(4);
		}
	}
	printf("Success!\n");
	return 0;
}
