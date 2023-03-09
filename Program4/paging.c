/*  paging.c
 *  Program 4, CPSC 3220
 *  Cameron L. Burroughs
 *
 *  simulation program to work with paging.cfg file
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// STRUCT DEFINITIONS
typedef struct {
    unsigned char presence;   /* could be single bit */
    unsigned short pad;
    unsigned char pfn;        /* 8 bits */
} PTE_t;

typedef struct {
    unsigned char valid;      /* could be single bit */
    unsigned short vpn;       /* 16 bits */
    unsigned char pfn;        /* 8 bits */
} TLBE_t;

typedef struct { 
    unsigned char valid;      /* could be single bit */
    unsigned char use_vector; /* 8 bits for pseudo-LRU replacement */
    unsigned short vpn;       /* 16 bits */
} CME_t;

// GLOBAL VARIABLES
bool verbose = false;
PTE_t* PT;
TLBE_t* TLB;
CME_t* CM;
int num_page_frames, num_tlb_entries, use_bit_period, tlb_index;


// FUNCTION PROTOTYPES
void print_stats(int ac, int tmc, int pfc);
void print_display(void);
void update_tlb(unsigned int vpn, unsigned int pfn);
void record_use(unsigned int pfn);
void update_use_vector(void);

int main(int argc, char** argv) {

    int access_count, tlb_miss_count, page_fault_count;
    unsigned int virtual_addr, page_table_entries;

    access_count = tlb_miss_count = page_fault_count = tlb_index = 0;
    page_table_entries = 0x10000;

    // getting command line options
    int c;
    for(c = 0; c < argc; c++) {
        if(strcmp(argv[c],"-v") == 0)
            verbose = true;
    }

    // opening and reading from file
    char* filename = "paging.cfg";
    FILE* fp = fopen(filename, "r");
    int rc;
    if(fp == NULL) {
        printf( "error in opening paging.config file\n" );
        return 0;
    }

    rc = fscanf(fp, "PF %d\n", &num_page_frames);
    if(rc != 1) {
        printf("error in reading PF (page frames) from configuration file\n");
        exit(-1);
    }

    rc = fscanf(fp, "TE %d\n", &num_tlb_entries);
    if(rc != 1) {
        printf( "error in reading TE (TLB entries) from configuration file\n" );
        exit(-1);
    }

    rc = fscanf(fp, "UP %d\n", &use_bit_period);
    if(rc != 1) {
    printf( "error in reading UP (use bit period) from configuration file\n" );
        exit(-1);
    }

    if(verbose) {
        printf( "paging simulation\n" );
        printf( "  %d virtual pages in the virtual address space\n", page_table_entries );
        printf( "  %d physical page frames\n", num_page_frames );
        printf( "  %d TLB entries\n", num_tlb_entries );
        printf( "  use vectors in core map are shifted every %d accesses\n\n", use_bit_period );
    }


    TLB = (TLBE_t*)calloc(num_tlb_entries, sizeof(TLBE_t));
    PT = (PTE_t*)calloc(page_table_entries,sizeof(PTE_t));
    CM = (CME_t*)calloc(num_page_frames, sizeof(CME_t));

    // reading vpn inputs 
    while((fscanf(stdin,"%x\n",&virtual_addr)) == 1) {
        if((virtual_addr & 0xFF000000) != 0x00000000) {
            print_stats(access_count, tlb_miss_count, page_fault_count);
            print_display();
            printf("\n");
            continue;
        }

        int hit = 0;
        unsigned short int page_num, offset;
        unsigned int phys_addr;
        
        access_count++;
        offset = virtual_addr & 0x00FF;
        page_num = virtual_addr >> 8;

        if(verbose) {
            printf( "access %d:\n", access_count );
            printf( "  virtual address is              0x%06x\n", virtual_addr );
        }

        //TLB lookup
        for(int i = 0; i < num_tlb_entries && !hit; i++) {
            if(TLB[i].vpn == page_num && TLB[i].valid) {
                hit = 1;
                record_use(TLB[i].pfn);
                phys_addr = (TLB[i].pfn << 8) | offset;
                if(verbose) {
                    printf( "  tlb hit, physical address is      0x%04x\n", phys_addr );
                }
            }
        }

        if(!hit) {
            tlb_miss_count++;
            if(verbose) {
                printf( "  tlb miss\n" );
            }
        }

        //PTE lookup if !hit
        if(!hit && PT[page_num].presence) {
            hit = 1;
            record_use(PT[page_num].pfn);
            phys_addr = (PT[page_num].pfn << 8) | offset;
            if(verbose) {
                printf( "  page hit, physical address is     0x%04x\n", phys_addr );
            }
            
            //TLB replacement
            update_tlb(page_num, PT[page_num].pfn);

        } else if(!hit) {
            page_fault_count++;
            if(verbose) {
                printf( "  page fault\n" );
            }
            int free_found = 0;
            //check for free frames
            for(int j = 0; j < num_page_frames && !free_found; j++) {
                if(CM[j].valid == 0) {                  // free frame found
                    record_use(j);
                    CM[j].valid = 1;
                    CM[j].vpn = page_num;
                    PT[page_num].presence = 1;
                    PT[page_num].pfn = j;
                    phys_addr = (j << 8) | offset;
                    if(verbose) { 
                        printf("  unused page frame allocated\n" ); 
                        printf( "  physical address is               0x%04x\n", phys_addr );
                    }   
                    free_found = 1;
                    update_tlb(page_num, j);
                }
            }
            if(!free_found) {
                if(verbose) { printf("  page replacement needed\n" ); }
                int low_pfn, i, old_vpn;
                low_pfn = 0;
                //find which to replace
                for( i = 1; i < num_page_frames; i++ ) {
                    if(CM[i].use_vector < CM[low_pfn].use_vector) {
                        low_pfn = i;
                    }
                }
                record_use(low_pfn);
                if(verbose) { printf( "  replace frame %d\n", low_pfn ); }
                // Update core map and page table
                old_vpn = CM[low_pfn].vpn;
                CM[low_pfn].vpn = page_num;
                CM[low_pfn].use_vector = 0x0000;
                record_use(low_pfn);
                PT[old_vpn].presence = 0;
                PT[old_vpn].pfn = 0x00;
                PT[page_num].presence = 1;
                PT[page_num].pfn = low_pfn;
                if(verbose) {printf("  TLB invalidate of vpn 0x%x\n", old_vpn);}

                int entry_found = 0;
                for(i = 0; i < num_tlb_entries && !entry_found; i++) {
                    if(TLB[i].pfn == low_pfn) {
                        TLB[i].valid = 0;
                        entry_found = 1;
                    }
                }
                phys_addr = (low_pfn << 8) | offset;
                if(verbose) {printf( "  physical address is               0x%04x\n", phys_addr );}

                update_tlb(page_num,low_pfn);
            }
        }
        if( ( access_count % use_bit_period ) == 0 ){
            if( verbose ) printf( "shift use vectors\n" );
            update_use_vector();
        }
    }

    if(verbose) { printf("\n"); }
    print_stats(access_count, tlb_miss_count, page_fault_count);
    if(verbose) { print_display(); }
    
    free(TLB);
    free(PT);
    free(CM);
    
    return 0;
}

void update_use_vector(void) {
    for(int i = 0; i < num_page_frames; i++) {
        CM[i].use_vector = CM[i].use_vector >> 1;
    }
}

void record_use( unsigned int pfn ){
    /* set high bit in use vector */
    CM[pfn].use_vector = CM[pfn].use_vector | 0x80;
}

void update_tlb(unsigned int vpn, unsigned int pfn) {
    int i, empty_found;
    i = empty_found = 0;
    for(i = 0; i < num_tlb_entries && !empty_found; i++) {
        if(TLB[i].valid == 0){
            empty_found = 1;
        }
    } i--;

    if(!empty_found) { i = tlb_index % num_tlb_entries; }
 
    TLB[i].vpn = vpn;
    TLB[i].pfn = pfn;
    TLB[i].valid = 1;

    tlb_index++;

    if(verbose) {
        printf("  tlb update of vpn 0x%04x with pfn 0x%02x\n", TLB[i].vpn, TLB[i].pfn );
    }
}

void print_stats(int ac, int tmc, int pfc) {
    printf( "statistics\n" );
    printf( "  accesses    = %d\n", ac);
    printf( "  tlb misses  = %d\n", tmc);
    printf( "  page faults = %d\n", pfc);
}

void print_display(void) {
    printf( "\ntlb\n" );
    for(int i = 0; i < num_tlb_entries; i++) {
       printf( "  valid = %x, vpn = 0x%04x, pfn = 0x%02x\n", TLB[i].valid, TLB[i].vpn, TLB[i].pfn); 
    }
    printf( "\ncore map table\n" );
    for(int i = 0; i < num_page_frames; i++) {
        printf( "  pfn = 0x%02x: valid = %d, use vector = 0x%02x, vpn = 0x%04x\n", i, CM[i].valid, CM[i].use_vector, CM[i].vpn);
    }
    printf( "\nfirst ten entries of page table\n" );
    for(int i = 0; i < 10; i++) {
        printf( "  vpn = 0x%04x: presence = %d, pfn = 0x%02x\n", i, PT[i].presence, PT[i].pfn);
    }
}
