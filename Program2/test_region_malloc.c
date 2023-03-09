#include "region.h"

typedef struct{
  region_t region;
  int global_count;
} protected_count_t;


void *worker( void *ptr ){
  protected_count_t *pc_ptr = (protected_count_t *) ptr;
  int local;

  enter_region( &(pc_ptr->region), true_predicate );
  printf( "thread inside region\n" );

  local = pc_ptr->global_count;
  sleep( 1 );

  local++;
  sleep( 1 );

  pc_ptr->global_count = local;

  printf( "thread leaving region\n" );
  exit_region( &(pc_ptr->region) );

  return NULL;
}

int main(){
  pthread_t threads[2];
  int rc;

  protected_count_t *ptr =
    (protected_count_t *) malloc( sizeof( protected_count_t ) );
  if( ptr == NULL ){ printf( "** could not malloc\n" ); exit( -1 ); }

  init_region( &(ptr->region), 1 );
  ptr->global_count = 0;

  rc = pthread_create( &threads[0], NULL, &worker, (void *)ptr );
  if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }
  rc = pthread_create( &threads[1], NULL, &worker, (void *)ptr );
  if( rc ){ printf( "** could not create thread\n" ); exit( -1 ); }

  rc = pthread_join(threads[0], NULL);
  if( rc ){ printf("** could not join thread\n" ); exit( -1 ); }
  rc = pthread_join(threads[1], NULL);
  if( rc ){ printf("** could not join thread\n" ); exit( -1 ); }

  printf( "global count is %d\n", ptr->global_count );

  finalize_region( &(ptr->region) );
  free( ptr );
  ptr = NULL;

  return 0;
}
