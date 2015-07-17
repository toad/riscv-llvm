#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tag.h"

#ifndef LENGTH
#define LENGTH 3
#else
# if LENGTH < 3
#  error LENGTH must be at least 3
# endif
#endif

int main() {
	printf("Testing overlapped memmove with LENGTH=%d\n", LENGTH);
	long realarray[LENGTH + 2];
	long* arr=&realarray[1];
	store_tag(&realarray[0], READ_ONLY);
	store_tag(&realarray[LENGTH+1], READ_ONLY);
	int i=0;
	for(i=0;i<LENGTH;i++) arr[i] = i;
	store_tag(&arr[0], LAZY);
	store_tag(&arr[1], LAZY);
	printf("Before move:\n");
        for(i=0;i<LENGTH+2;i++) {
                printf("Array[%d]=%ld tag %d at %p\n", i, realarray[i], load_tag(&realarray[i]), &realarray[i]);
	}
	printf("\n");
	memmove(&arr[1], &arr[0], sizeof(long)*(LENGTH-1));
	for(i=0;i<LENGTH+2;i++) {
		printf("Array[%d]=%ld tag %d\n", i, realarray[i], load_tag(&realarray[i]));
	}
	for(i=1;i<LENGTH;i++) {
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
	for(i=0;i<LENGTH+2;i++) store_tag(&realarray[i], 0);
	printf("Success!\n\n");
	return 0;
}
