#include "region.h"

/* region definitions */

  region_t region[2];

  /* each region entry protects a global count */

  int global_count[2];

  /* there are no predicate functions other than */
  /*   true_predicate(), which is defined in the */
  /*   header file                               */

/* end */


void *worker_0( void *thread_id ){
  int local;

  enter_region( &region[0], true_predicate );
  printf( "thread %ld inside region 0\n", (long)thread_id );

  local = global_count[0];
  sleep( 1 );

  local++;
  sleep( 1 );

  global_count[0] = local;

  printf( "thread %ld leaving region 0\n", (long)thread_id );
  exit_region( &region[0] );

  return NULL;
}

void *worker_1( void *thread_id ){
  int local;

  enter_region( &region[1], true_predicate );
  printf( "thread %ld inside region 1\n", (long)thread_id );

  local = global_count[1];
  sleep( 1 );

  local++;
  sleep( 1 );

  global_count[1] = local;

  printf( "thread %ld leaving region 1\n", (long)thread_id );
  exit_region( &region[1] );

  return NULL;
}

int main(){
  pthread_t threads[6];
  int rc;
  int i;

  /* initializations */

  init_region( &region[0], 1 );
  init_region( &region[1], 1 );
  global_count[0] = 0;
  global_count[1] = 0;


  /* create three worker threads to update global count 0 */

  for( i = 0; i < 3; i++ ){
    rc = pthread_create( &threads[i], NULL, &worker_0, (void *)((long)i) );
    if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }
  }

  /* create three worker threads to update global count 1 */

  for( i = 3; i < 6; i++ ){
    rc = pthread_create( &threads[i], NULL, &worker_1, (void *)((long)i) );
    if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }
  }


  /* join all threads */

  for( i = 0; i < 6; i++ ){
    rc = pthread_join(threads[i], NULL);
    if( rc ){ printf("** could not join thread\n" ); exit( -1 ); }
  }


  /* display the updated counts */

  printf( "global count[0] is %d\n", global_count[0] );
  printf( "global count[1] is %d\n", global_count[1] );


  /* clean up and end the program */

  finalize_region( &region[0] );
  finalize_region( &region[1] );

  return 0;
}
