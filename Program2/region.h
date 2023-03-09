#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

typedef struct{
  unsigned signature;
  int count;
  int max_count;
  pthread_mutex_t lock; 
  pthread_cond_t delay_cv; 
  pthread_t *inside_region_list;
} region_t;

void init_region( region_t *r, int max );
void enter_region( region_t *r, int (* predicate)() );
void exit_region( region_t *r );
void finalize_region( region_t *r );
int true_predicate();
