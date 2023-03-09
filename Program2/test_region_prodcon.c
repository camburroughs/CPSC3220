#include "region.h"

/* region definitions */

  region_t buffer_region;

  /* the buffer_region protects these variables */

  long int single_item_buffer;  /* producer thread will store its own id */
  int item_available;           /* logical flag, 0 == false, 1 == true   */

  /* the buffer_region has these predicate functions */

  int item_is_available(){ return item_available; }
  int item_can_be_placed(){ return !item_available; }

/* end */


void *producer( void *thread_id ){
  pthread_t pthread_id = pthread_self();
  enter_region( &buffer_region, item_can_be_placed );
  printf( "thread %ld inside region and places parent-assigned id %ld\n",
    (long)thread_id, (long)thread_id );
  printf( "  note: pthread id for thread %ld is actually %ld\n",
    (long int) thread_id, (long int) pthread_id );

  single_item_buffer = (long) thread_id;
  item_available = 1;
  sleep( 1 );

  printf( "thread %ld leaving region\n", (long)thread_id );
  exit_region( &buffer_region );

  return NULL;
}

void *consumer( void *thread_id ){
  enter_region( &buffer_region, item_is_available );
  printf( "thread %ld inside region and obtains %ld\n",
    (long)thread_id, single_item_buffer );

  single_item_buffer = -1;
  item_available = 0;
  sleep( 1 );

  printf( "thread %ld leaving region\n", (long)thread_id );
  exit_region( &buffer_region );

  return NULL;
}


/* test driver */

int main(){
  pthread_t threads[6];
  int rc;
  int i;

  /* initialization */

  init_region( &buffer_region, 1 );
  single_item_buffer = -1;
  item_available = 0;


  /* create three producer threads and three consumer threads */

  for( i = 0; i < 3; i++ ){
    rc = pthread_create( &threads[i], NULL, &producer, (void *)((long)i) );
    if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }
  }

  for( i = 3; i < 6; i++ ){
    rc = pthread_create( &threads[i], NULL, &consumer, (void *)((long)i) );
    if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }
  }


  /* join all threads */

  for( i = 0; i < 6; i++ ){
    rc = pthread_join(threads[i], NULL);
    if( rc ){ printf("** could not join thread\n" ); exit( -1 ); }
  }


  /* clean up and end the program */

  finalize_region( &buffer_region );

  return 0;
}
