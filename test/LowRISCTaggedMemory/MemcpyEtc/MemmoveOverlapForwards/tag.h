// See LICENSE for license details.

#ifndef LOWRISC_TAG_H
#define LOWRISC_TAG_H

#define TAG_WIDTH 4

#define LAZY 4
#define READ_ONLY 1
#define INVALID 3

static inline int load_tag(const void *addr) {
  int rv = 32;
  asm volatile ("ltag %0, 0(%1)"
                :"=r"(rv)
                :"r"(addr)
                );
  return rv;
}


static inline void store_tag(void *addr, int tag) {
  asm volatile ("stag %0, 0(%1)"
                :
                :"r"(tag), "r"(addr)
                );
}

#endif
