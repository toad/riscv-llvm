// See LICENSE for license details.

#ifndef LOWRISC_TAG_H
#define LOWRISC_TAG_H

#define TAG_WIDTH 4

#define LAZY_TAG 4

/* FIXME these are not inline because it breaks clang if they are! */

int load_tag(void *addr);

void store_tag(void *addr, int tag);

#endif
