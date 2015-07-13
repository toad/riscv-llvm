#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	void (*fn)(void);
	int x;
} Object;

Object *createEvilObject();

Object *copyObject(Object *);
