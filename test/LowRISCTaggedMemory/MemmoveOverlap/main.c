#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tag.h"

#define LAZY 4

void main() {
	long arr[] = { 1, 2, 3 };
	store_tag(&arr[0], LAZY);
	store_tag(&arr[1], LAZY);
	memmove(&arr[1], &arr[0], sizeof(long)*2);
	if(arr[1] != 1 && arr[2] != 2) {
		printf("ERROR: array is now %ld %ld %ld\n", arr[0], arr[1], arr[2]);
		exit(1);
	}
	if(load_tag(&arr[1]) != LAZY) {
		printf("ERROR: Tag on [1] should be LAZY=%d, is %d", LAZY, load_tag(&arr[1]));
		exit(2);
	}
	if(load_tag(&arr[2]) != LAZY) {
		printf("ERROR: Tag on [2] should be LAZY=%d, is %d", LAZY, load_tag(&arr[2]));
		exit(3);
	}
	printf("Success!\n");
}
