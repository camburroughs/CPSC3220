#include "region.h"
#define MAX_THREADS 9

/* region definition */

  region_t multiplex_region;
  region_t mutex_region;

  int present[MAX_THREADS];

  /* helper function */

  int inside_region(){
    int i;
    int sum = 0;
    for( i = 0; i < MAX_THREADS; i++ ){
      sum += present[i];
    }
    return sum;
  }

/* end */


void *worker( void *thread_id ){
  long int i = (long) thread_id; 
  int j = (int) i;

  enter_region( &multiplex_region, true_predicate );

  enter_region( &mutex_region, true_predicate );
  present[j] = 1;
  printf( "thread %ld inside region with thread count %d\n",
    (long)thread_id, inside_region() );
  exit_region( &mutex_region );

  sleep( 1 );

  printf( "thread %ld leaving region\n", (long)thread_id );

  enter_region( &mutex_region, true_predicate );
  present[j] = 0;
  exit_region( &mutex_region );

  exit_region( &multiplex_region );

  return NULL;
}


int main(){
  pthread_t threads[MAX_THREADS];
  int rc;
  int i;

  init_region( &multiplex_region, 3 );
  init_region( &mutex_region, 1 );

  for( i = 0; i < MAX_THREADS; i++ ){
    present[i] = 0;
  }

  for( i = 0; i < MAX_THREADS; i++ ){
    rc = pthread_create( &threads[i], NULL, &worker, (void *)((long)i) );
    if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }
  }

  for( i = 0; i < MAX_THREADS; i++ ){
    rc = pthread_join(threads[i], NULL);
    if( rc ){ printf("** could not join thread\n" ); exit( -1 ); }
  }

  finalize_region( &multiplex_region );
  finalize_region( &mutex_region );

  return 0;
}
