#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	void (*fn)(void);
	int x;
#if ARRAY_SIZE > 0
	long array[ARRAY_SIZE];
#endif
} Object;

Object *createEvilObject();

Object *copyObject(Object *);

Object *moveObject(Object *);

#if ARRAY_SIZE > 0
void fillArray(Object* p);
void checkArray(Object* p);
#else
#define fillArray(blah) blah
#define checkArray(blah) blah
#endif
