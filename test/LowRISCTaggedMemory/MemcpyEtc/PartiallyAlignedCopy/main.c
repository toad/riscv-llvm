/* Test partial alignment. That is, memcpy/memmove where source and dest are 8-byte
 * aligned but the number of bytes copied may not be. The number of bytes copied is
 * a compile-time parameter, so for small numbers of bytes this tests the compiler.
 * For large numbers of bytes copied it tests libc. */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifndef NO_TAGS
# ifndef __TAGGED_MEMORY__
#  error Tagged memory must be supported by compiler!
# endif
#include <sys/platform/tag.h>
#define load_tag __riscv_load_tag
#define store_tag __riscv_store_tag
#define LAZY __RISCV_TAG_LAZY
#define READ_ONLY __RISCV_TAG_READ_ONLY
#endif

void verify(long *aa, long *bb, int length, int words) {
	printf("Verifying length %d words %d\n", length, words);
	int i;
	for(i=0;i<words;i++) {
		assert(aa[i] == i);
		assert(((unsigned char)load_tag(&aa[i])) == ((unsigned char)(i << 3)));
	}
	if(length % sizeof(long) == 0) {
		printf("Copied whole words, should have copied tags...\n");
		for(i=0;i<words;i++) {
			assert(bb[i] == i);
			assert(((unsigned char)load_tag(&bb[i])) == ((unsigned char)(i << 3)));
		}
	} else {
		printf("Copied bytes, should NOT have copied tags...\n");
		for(i=0;i<words;i++) {
			printf("aa[%d] = %ld tag %d\n", i, aa[i], load_tag(&aa[i]));
			printf("bb[%d] = %ld tag %d\n", i, bb[i], load_tag(&bb[i]));
		}
		for(i=0;i<words;i++) {
			assert(load_tag(&bb[i]) == 0);
			if(i != words-1) assert(bb[i] == aa[i]);
		}
	}
}

int main(int argc, char **argv) {
	int WORDS = (LENGTH + sizeof(long) - 1) / 8;
	int WORDS_WITH_BUMPERS = WORDS+2;
	printf("Doing unaligned copy test with length %d and %d words\n", LENGTH, WORDS);
	long *a = malloc(sizeof(long)*WORDS_WITH_BUMPERS);
	long *b = malloc(sizeof(long)*WORDS_WITH_BUMPERS);
	printf("Allocated space\n");
	store_tag(&a[0], __RISCV_TAG_INVALID);
	store_tag(&b[0], __RISCV_TAG_INVALID);
	printf("Added bumpers at start\n");
	store_tag(&a[WORDS+1], __RISCV_TAG_INVALID);
	store_tag(&b[WORDS+1], __RISCV_TAG_INVALID);
	printf("Added bumpers at end\n");
	long *aa = &a[1];
	long *bb = &b[1];
	int i;
	for(i=0;i<WORDS;i++) {
		aa[i] = i;
		bb[i] = 0;
		store_tag(&aa[i], i << 3);
	}
	printf("Copying.\n");
	memcpy(bb, aa, LENGTH);
	verify(aa, bb, LENGTH, WORDS);
	for(i=0;i<WORDS;i++) {
		bb[i] = 0;
		store_tag(&bb[i], 0);
	}
	memmove(bb, aa, LENGTH);
	verify(aa, bb, LENGTH, WORDS);
	for(i=0;i<WORDS_WITH_BUMPERS;i++) {
		store_tag(&a[i], 0);
		store_tag(&b[i], 0);
	}
	printf("Success!\n");
}

