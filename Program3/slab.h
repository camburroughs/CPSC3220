#include <stdio.h>

typedef struct{
  short unsigned int free_count;
  short unsigned int free_mask;
  unsigned int signature;
  unsigned char free_space[4088];
} slab_t; 

extern slab_t __attribute__ ((aligned (65536))) s[16];
extern unsigned char *start;
extern short unsigned int empty_mask, partial_mask, full_mask;

unsigned char *slab_allocate();
int slab_release( unsigned char *addr );
