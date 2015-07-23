// See LICENSE for license details.

#ifndef LOWRISC_TAG_H
#define LOWRISC_TAG_H

#define TAG_WIDTH 4
#define LAZY_TAG 4
#define READ_ONLY 1

/* FIXME These are not inline because at -O0, neither gcc nor clang is happy
 * if they are! clang doesn't emit it when building, even when building tag.c,
 * and gcc emits it twice and gets a linker error. */

int load_tag(void *addr) {
  int rv = 32;
  asm volatile ("ltag %0, 0(%1)"
                :"=r"(rv)
                :"r"(addr)
                );
  return rv;
}


void store_tag(void *addr, int tag) {
  asm volatile ("stag %0, 0(%1)"
                :
                :"r"(tag), "r"(addr)
                );
}

#endif
