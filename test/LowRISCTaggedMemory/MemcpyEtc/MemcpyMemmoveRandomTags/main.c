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
#define LAZY __RISCV_TAG_CLEAN_FPTR
#define READ_ONLY __RISCV_TAG_READ_ONLY
#endif

/* Tag mask. You need to clear bits that affect behaviour, and bits that aren't stored.
 * Bottom two bits cause traps. */
#define TAG_MASK (((1 << TAG_WIDTH) - 1) & ~3)

void dump(long *aa, int length) {
	int i;
	for(i=0;i<length;i++) {
		printf("[%d] = %ld, tag %d\n", i, aa[i], (unsigned char)load_tag(&aa[i]));
	}
}

void verify(int seed, long *aa, int length) {
	srand(seed);
	int i;
//	printf("Verify with tags:\n");
//	dump(aa, length);
	for(i=0;i<length;i++) {
		long randomValue = rand();
		unsigned char randomTag = rand() & TAG_MASK;
		assert(aa[i] == randomValue);
		assert((unsigned char)load_tag(&aa[i]) == randomTag);
	}
}

void verifyNoTags(int seed, long *aa, int length) {
	srand(seed);
	int i;
	for(i=0;i<length;i++) {
		long randomValue = rand();
		unsigned char randomTag = rand() & TAG_MASK;
//		printf("[%d] = %ld, tag %d\n", i, aa[i], (unsigned char)load_tag(&aa[i]));
		assert(aa[i] == randomValue);
		assert((unsigned char)load_tag(&aa[i]) == 0);
	}
}

void clean(long *aa, int length) {
	int i;
	for(i=0;i<length;i++,aa++) {
		store_tag(aa, 0);
		*aa = 0;
	}
}

int main(int argc, char **argv) {
	printf("Doing random tag test with length %d tag mask %d\n", LENGTH, TAG_MASK);
	long *a = malloc(sizeof(long)*(LENGTH+2));
	long *b = malloc(sizeof(long)*(LENGTH+2));
	printf("Allocated space\n");
	store_tag(&a[0], __RISCV_TAG_INVALID);
	store_tag(&b[0], __RISCV_TAG_INVALID);
	printf("Added bumpers at start\n");
	store_tag(&a[LENGTH+1], __RISCV_TAG_INVALID);
	store_tag(&b[LENGTH+1], __RISCV_TAG_INVALID);
	printf("Added bumpers at end\n");
	long *aa = &a[1];
	long *bb = &b[1];
	int seed = rand();
	srand(seed);
	int i;
	printf("Setting up...\n");
	for(i=0;i<LENGTH;i++) {
		long randomValue = rand();
		char randomTag = rand() & TAG_MASK;
		aa[i] = randomValue;
		store_tag(&aa[i], randomTag);
//		printf("[%d] = %ld, tag %d\n", i, randomValue, randomTag);
	}
	// Should copy tags.
	printf("Calling memcpy\n");
	memcpy(bb, aa, LENGTH * sizeof(long));
	verify(seed, aa, LENGTH);
	verify(seed, bb, LENGTH);
	clean(bb, LENGTH);
	printf("Calling memmove\n");
	memmove(bb, aa, LENGTH * sizeof(long));
	verify(seed, aa, LENGTH);
	verify(seed, bb, LENGTH);
	clean(bb, LENGTH);
	printf("Calling __riscv_memcpy_tagged\n");
//	printf("aa:\n");
//	dump(aa, LENGTH);
//	printf("bb:\n");
//	dump(bb, LENGTH);
	__riscv_memcpy_tagged(bb, aa, LENGTH * sizeof(long));
//	printf("After __riscv_memcpy_tagged:\n");
//	printf("aa:\n");
//	dump(aa, LENGTH);
//	printf("bb:\n");
//	dump(bb, LENGTH);
	verify(seed, aa, LENGTH);
	verify(seed, bb, LENGTH);
	clean(bb, LENGTH);
	printf("Calling __riscv_memmove_tagged\n");
	__riscv_memmove_tagged(bb, aa, LENGTH * sizeof(long));
	verify(seed, aa, LENGTH);
	verify(seed, bb, LENGTH);
	clean(bb, LENGTH);
	printf("Calling __riscv_memcpy_no_tags\n");
	__riscv_memcpy_no_tags(bb, aa, LENGTH * sizeof(long));
	verify(seed, aa, LENGTH);
	verifyNoTags(seed, bb, LENGTH);
	printf("Calling __riscv_memmove_no_tags\n");
	__riscv_memmove_no_tags(bb, aa, LENGTH * sizeof(long));
	verify(seed, aa, LENGTH);
	verifyNoTags(seed, bb, LENGTH);
	memmove(bb, aa, LENGTH * sizeof(long));
#if LENGTH > 1
	printf("Moving forward with memmove in-place by one slot...\n");
	memmove(&bb[1], &bb[0], (LENGTH-1) * sizeof(long));
	verify(seed, &bb[1], LENGTH-1);
	verify(seed, &bb[0], 1);
	printf("Cleaning...\n");
	memmove(bb, aa, LENGTH * sizeof(long));
	verify(seed, aa, LENGTH);
	verify(seed, bb, LENGTH);
	printf("Moving backwards with memmove in-place by one slot...\n");
	memmove(&bb[0], &bb[1], (LENGTH-1)*sizeof(long));
	verify(seed, aa, LENGTH);
	assert(bb[LENGTH-1] == aa[LENGTH-1]);
	assert(load_tag(&bb[LENGTH-1]) == load_tag(&aa[LENGTH-1]));
//	for(i=0;i<LENGTH-1;i++) {
//		printf("bb[%d] = %ld, tag %d should be %ld, tag %d\n", i, bb[i],
//                  (unsigned char)load_tag(&bb[i]), aa[i+1], 
//                  (unsigned char)load_tag(&aa[i+1]));
//	}
	for(i=0;i<LENGTH-1;i++) {
		assert(bb[i] == aa[i+1]);
		assert(load_tag(&bb[i]) == load_tag(&aa[i+1]));
	}
#endif /* LENGTH > 1 */
	clean(a, LENGTH+2);
	clean(b, LENGTH+2);
	printf("Success!\n");
}

