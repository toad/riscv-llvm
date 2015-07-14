#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	void (*fn)(void);
	int x;
	long array[ARRAY_SIZE];
} Object;

Object *createEvilObject();

Object *copyObject(Object *);

Object *moveObject(Object *);
