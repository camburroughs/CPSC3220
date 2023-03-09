#include "slab.h"

slab_t __attribute__ ((aligned (65536))) s[16];
unsigned char *start;
short unsigned int empty_mask, partial_mask, full_mask;

void init_slabs(){
  int i;
  empty_mask = 0xffff;
  partial_mask = full_mask = 0;
  for(i=0;i<16;i++){
    s[i].free_count = 15;
    s[i].free_mask = 0x7fff;
    s[i].signature = 0x51ab51ab;
  }
  start = (unsigned char *)s;
}

void slab_inspect( unsigned char *addr ){
  unsigned int sanity_check = ((unsigned int)(addr - start))>>16;
  unsigned int slab_index = ((unsigned int)(addr - start))>>12;
  // unsigned int obj_index = (((unsigned int)(addr - start))>>8)&15;
  if(sanity_check)
    printf("range check error in inspect address!\n");
  else if(s[slab_index].signature!=0x51ab51ab)
    printf("signature check error in inspect address!\n");
  else
    printf("slab %d has free count of %d and free mask of %04x\n",
      slab_index,
      s[slab_index].free_count,
      (unsigned int)s[slab_index].free_mask);
}


int main(){
  unsigned char *address[256];
  int rc;
  unsigned char *addr;

  init_slabs();

  printf("start addr is %p\n",start);
  printf("empty/partial/full = %04x %04x %04x\n", (unsigned int)empty_mask,
    (unsigned int)partial_mask, (unsigned int)full_mask);

  address[1] = slab_allocate();
  printf("allocated addr is %p\n",address[1]);
  slab_inspect(address[1]);

  addr = address[1];
  addr = (unsigned char *)((unsigned long long int)addr >> 12);
  addr = (unsigned char *)((unsigned long long int)addr << 12);
  printf("%p: 0x%02x\n",addr,*addr);
  addr++;
  printf("%p: 0x%02x\n",addr,*addr);
  addr++;
  printf("%p: 0x%02x\n",addr,*addr);
  addr++;
  printf("%p: 0x%02x\n",addr,*addr);
  addr++;
  printf("%p: 0x%02x\n",addr,*addr);
  addr++;
  printf("%p: 0x%02x\n",addr,*addr);
  addr++;
  printf("%p: 0x%02x\n",addr,*addr);
  addr++;
  printf("%p: 0x%02x\n",addr,*addr);

  printf("empty/partial/full = %04x %04x %04x\n", (unsigned int)empty_mask,
    (unsigned int)partial_mask, (unsigned int)full_mask);

  address[2] = slab_allocate();
  printf("allocated addr is %p\n",address[2]);
  address[3] = slab_allocate();
  address[4] = slab_allocate();
  address[5] = slab_allocate();
  address[6] = slab_allocate();
  address[7] = slab_allocate();
  address[8] = slab_allocate();
  address[9] = slab_allocate();
  address[10] = slab_allocate();
  address[11] = slab_allocate();
  address[12] = slab_allocate();
  address[13] = slab_allocate();
  address[14] = slab_allocate();

  printf("empty/partial/full = %04x/%04x/%04x\n", (unsigned int)empty_mask,
    (unsigned int)partial_mask, (unsigned int)full_mask);

  address[15] = slab_allocate();

  printf("empty/partial/full = %04x/%04x/%04x\n", (unsigned int)empty_mask,
    (unsigned int)partial_mask, (unsigned int)full_mask);

  address[16] = slab_allocate();

  printf("empty/partial/full = %04x/%04x/%04x\n", (unsigned int)empty_mask,
    (unsigned int)partial_mask, (unsigned int)full_mask);

  slab_inspect(address[6]);
  rc = slab_release(address[6]);
  printf("return code is %d, expecting 0\n",rc);
  slab_inspect(address[6]);
  rc = slab_release(address[6]);
  printf("return code is %d, expecting 2\n",rc);
  rc = slab_release(0);
  printf("return code is %d, expecting 1\n",rc);
  rc = slab_release(start-256);
  printf("return code is %d, expecting 1\n",rc);

  printf("empty/partial/full = %04x/%04x/%04x\n", (unsigned int)empty_mask,
    (unsigned int)partial_mask, (unsigned int)full_mask);

  address[17] = slab_allocate();
  slab_inspect(address[17]);

  printf("empty/partial/full = %04x/%04x/%04x\n", (unsigned int)empty_mask,
    (unsigned int)partial_mask, (unsigned int)full_mask);
  
  return 0;
}
