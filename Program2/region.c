/* region.c 
 * CPSC 3220 Program 2
 * Cameron L. Burroughs
 */

#include "region.h"
#define TRUE 1
#define FALSE 0


//called by a parent fn before creating child threads, no lock needed
void init_region( region_t *r, int max )
{
    r->signature = 0xc1c0ffee;
    r->count = 0;
    r->max_count = max;
    //fn gathered from pthread library
    if(pthread_mutex_init(&r->lock, NULL) != 0) {
        printf("[!] MUTEX INITIALIZATION FAILED\n");
        return;
    }
    pthread_cond_init(&r->delay_cv, NULL);
    r->inside_region_list =(pthread_t*)calloc(r->max_count,sizeof(pthread_t));
}

void enter_region( region_t *r, int (* predicate)() )
{
    if(r->signature != 0xc1c0ffee){
	    printf("[!] thread trying to enter an uninitialized region\n");
	    pthread_exit(NULL);
    }
    //obtain mutex
    pthread_mutex_lock(&r->lock);
    //wait on condition variable ** in while loop **
    while(r->count == r->max_count || !predicate()) {
        pthread_cond_wait(&r->delay_cv, &r->lock);
    }
    //search for matching thread ID, if found exit
    int i = 0;
    for( ; i < r->max_count; i++ ) {
        if(pthread_equal(pthread_self(), r->inside_region_list[i])) {
            printf("[!] CALLING THREAD DUPLICATE IN REGION\n");
            pthread_mutex_unlock(&r->lock);
            pthread_exit(NULL);
        }
    }
    //add current thread ID to 1st empty spot in list
    i = 0;
    while(i < r->max_count && r->inside_region_list[i] != 0) {
        i++;
    }
    r->inside_region_list[i] = pthread_self();
    //increment count, release lock.
    r->count++;
    pthread_mutex_unlock(&r->lock);
    return;
}

void exit_region( region_t *r ) 
{
    //check signature
    if(r->signature != 0xc1c0ffee){
	    printf("[!] thread trying to enter an uninitialized region\n");
	    pthread_exit(NULL);
    }
    //obtain mutex
    pthread_mutex_lock(&r->lock);
    
    int i = 0;
    while( i < r->max_count && !pthread_equal(pthread_self(),r->inside_region_list[i])) {
        i++;
    }
    if(i == r->max_count) {
        printf("[!] THREAD NOT FOUND IN REGION\n");
        pthread_mutex_unlock(&r->lock);
        pthread_exit(NULL);
    } else { 
        r->inside_region_list[i] = 0;       //remove thread from list
        r->count--;
        pthread_cond_signal(&r->delay_cv);  //wake up threads
        pthread_mutex_unlock(&r->lock);     //release lock
        return;

    }
}
void finalize_region( region_t *r ) 
{
    if(r->signature != 0xc1c0ffee){
	    printf("[!] thread trying to enter an uninitialized region\n");
	    pthread_exit(NULL);
    }
    //destroy mutex and condition variable
    pthread_mutex_destroy(&r->lock);
    pthread_cond_destroy(&r->delay_cv);
    free(r->inside_region_list);
    if(r->count != 0)
        printf("[!] ONE OR MORE THREADS DID NOT EXIT PROPERLY\n");

}
int true_predicate(void) {
    return TRUE;
}

