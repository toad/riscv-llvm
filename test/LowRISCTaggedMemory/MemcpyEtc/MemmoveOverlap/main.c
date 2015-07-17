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
	long arr[LENGTH];
	int i=0;
	for(i=0;i<LENGTH;i++) arr[i] = i;
	store_tag(&arr[0], LAZY);
	store_tag(&arr[1], LAZY);
	printf("Before move:\n");
        for(i=0;i<LENGTH;i++) {
                printf("Array[%d]=%ld tag %d\n", i, arr[i], load_tag(&arr[i]));
	}
	memmove(&arr[1], &arr[0], sizeof(long)*(LENGTH-1));
	printf("Array[0]=%ld tag %d\n", arr[0], load_tag(&arr[0]));
	for(i=1;i<LENGTH;i++) {
		printf("Array[%d]=%ld tag %d should be %d\n", i, arr[i], load_tag(&arr[i]), i-1);
		if(arr[i] != i-1) {
			printf("Array[%d]=%ld should be %d\n", i, arr[i], i-1);
			printf("Dump:\n\n");
			for(i=0;i<LENGTH;i++) {
				printf("Array[%d] = %ld\n", i, arr[i]);
			}
			exit(4);
		}
	}
	if(load_tag(&arr[1]) != LAZY) {
		printf("ERROR: Tag on [1] should be LAZY=%d, is %d\n", LAZY, load_tag(&arr[1]));
		exit(2);
	}
	if(load_tag(&arr[2]) != LAZY) {
		printf("ERROR: Tag on [2] should be LAZY=%d, is %d\n", LAZY, load_tag(&arr[2]));
		exit(3);
	}
	printf("Success!\n");
	return 0;
}
