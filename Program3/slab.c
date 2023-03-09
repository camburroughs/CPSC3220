/* slab.c 
 * Program 3
 * Cameron Burroughs
 * CPSC 3220 SPRING 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include "slab.h"

#define sig 0x51ab51ab

unsigned char *find_first(short unsigned int mask, unsigned int offset);

unsigned char *slab_allocate(void)
{
    short unsigned int rover;
    unsigned char *slab_start, *obj_start;
    int index = 15;
    if(full_mask == 0xffff) {
        return NULL; 
    } else if(partial_mask != 0x0000) {
        //find first partial
        for(rover = 0x8000; rover > 0x0000; rover >> 1, index--) {
            if(rover & partial_mask) {
                //find address of corresponding slab
                slab_start = start + (index*0x1000);
                if(slab_start->free_count == 1) { //changes from 1 to 0
                    slab_start->free_count--;
                    partial_mask = partial_mask & ~rover; //clear bit in partial mask
                    full_mask = full_mask & rover; //set bit in full mask 
                }
                rover = 0x0000;
            }
        }
    } else {
        //find first empty
        for(rover = 0x8000; rover > 0x0000; rover >> 1, index--) {
            if(rover & empty_mask) {
                //find address of corresponding slab
                slab_start = start + (index*0x1000);
                if(slab_start->free_count == 15) { //TODO possibly remove condition, it is redundant 
                    slab_start->free_count--;
                    empty_mask = empty_mask & ~rover; //clear bit in empty mask
                    partial_mask = empty_mask & rover; //set bit in partial mask
                }
                rover = 0x0000;
            }
        }
    }
    //find first free object
    index = 15;
    for(rover = 0x8000; rover > 0x000; rover >> 1, index--) {
        if(rover & slab_start->free_mask) {
            obj_start = slab_start + (index*0x0100);
            slab_start->free_mask = slab_start->free_mask & ~rover; //clear bit in free mask
            rover = 0x0000;
        }
    }
    return obj_start;
}
int slab_release(unsigned char *addr)
{
    if( addr > (start + 4096) || addr->signature != sig)
        return 1;
}

/*unsigned char *find_first(unsigned char *begin, short unsigned int mask, unsigned int offset)
{
    short unsigned int rover;
    int index = 15;
    for(rover = 0x8000; rover > 0; rover >> 1, index--) {
        if(rover & mask) {
            return begin + (index*offset);
        }
    }
}*/
