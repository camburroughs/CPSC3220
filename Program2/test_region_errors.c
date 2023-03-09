#include "region.h"
#define MAX_THREADS 5

/* region definitions */

  region_t uninit_region;
  region_t mutex_region;
  region_t multiplex_region;
  region_t second_mutex_region;

  int test_count = 0;

/* end */

void *try_to_enter_uninit( void *thread_id ){
  enter_region( &uninit_region, true_predicate );
  return NULL;
}

void *try_to_exit_wrong_region( void *thread_id ){
  enter_region( &multiplex_region, true_predicate );
  exit_region( &mutex_region );
  return NULL;
}

void *try_to_enter_twice( void *thread_id ){
  enter_region( &multiplex_region, true_predicate );
  enter_region( &multiplex_region, true_predicate );
  return NULL;
}

void *set_test_count( void *thread_id ){
  sleep( 1 ); 
  enter_region( &second_mutex_region, true_predicate );
  test_count++;
  exit_region( &second_mutex_region );
  return NULL;
}


int main(){
  pthread_t threads[MAX_THREADS];
  int rc;
  int i;

  init_region( &mutex_region, 1 );
  init_region( &multiplex_region, 3 );
  init_region( &second_mutex_region, 1 );

  rc = pthread_create( &threads[0], NULL, &try_to_enter_uninit,
    (void *)((long)0) );
  if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }

  rc = pthread_create( &threads[1], NULL, &try_to_exit_wrong_region,
    (void *)((long)1) );
  if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }

  rc = pthread_create( &threads[2], NULL, &try_to_enter_twice,
    (void *)((long)2) );
  if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }

  rc = pthread_create( &threads[3], NULL, &set_test_count,
    (void *)((long)3) );
  if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }

  rc = pthread_create( &threads[4], NULL, &set_test_count,
    (void *)((long)4) );
  if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }

  for( i = 0; i < MAX_THREADS; i++ ){
    rc = pthread_join(threads[i], NULL);
    if( rc ){ printf("** could not join thread\n" ); exit( -1 ); }
  }

  finalize_region( &uninit_region );
  finalize_region( &mutex_region );
  finalize_region( &multiplex_region );
  finalize_region( &second_mutex_region );

  printf( "test count is %d\n", test_count );

  return 0;
}
