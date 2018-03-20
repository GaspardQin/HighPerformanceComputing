/*
  
  Simulating a very simple cache to estimate the number of 
  misses and hits  
  
  
  The principal functions to modify are :
    cache_fetchmemory(Cache *c, void *adr);
    cache_test();       
  
*/


/* different strategy to hash adress */
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define CACHE_DIRECT_MAPPING   0
#define CACHE_FULLYASSOCIATIVE 1
#define CACHE_2WAYASSOCIATIVE  2
#define CACHE_4WAYASSOCIATIVE  3


typedef struct t_cache
{
  int hits;
  int misses;
  
  int strategy;
  int nbrblocks;
  void **blocks; /* adress */
  int   *used;   /* store how many time the data was fetched */
  
  int accesstime; /* sum up access times to retrieve data (number of tests)*/
  
} Cache;
 

Cache * cache_init(int size, int strategy);
Cache * cache_free(Cache *c);
void    cache_reset(Cache *c);                  /* Reset all variables and removed of previously store addresses      */
int     cache_isvalid(Cache *c );               /* Verify that all arrays are properly allocated in stacks            */
void    cache_print(Cache *c, int level);       /* Print the cache, if Level > 1 all the stored adresses are reported */
void    cache_fetchmemory(Cache *c, void *adr); /* get an address and set the number of hits/misses, time of access   */
 

/* 

  Simple functions to evaluate the efficiency of the cache approach 
  Warning : for simplicity, arrays/matrices are allocated on stack, 
  to make experiments with large matrix, write function to dynalically allocate vectors/matrices 

*/
void cache_dotprod(int n, Cache *c);
void cache_matvec( int n, Cache *c);
void cache_matmat( int n, Cache *c);
void cache_dotprod_block(int n, Cache *c, int blocksize);
void cache_test(void);   /* main functions of tests */

