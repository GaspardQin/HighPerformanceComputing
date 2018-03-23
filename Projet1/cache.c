#include <cache.h>

/* initialization functions of vector/matrices with random and zero values */
static void data_randominit(int n, double *A);
static void data_zeroinit(int n, double *A); 


Cache * cache_init(int size, int strategy)
{ 
  int i;
  Cache *c = malloc(sizeof(Cache));
  if ( ! c ) return NULL;
  c->hits   = 0;
  c->misses = 0;
  
  c->strategy   = strategy ;
  c->nbrblocks  = size;
  c->blocks     = malloc(sizeof(void *) * size );
  for(i=0; i<c->nbrblocks; i++) c->blocks[i] = NULL;
  
  c->used     = malloc(sizeof(int) * size );
  for(i=0; i<c->nbrblocks; i++) c->used[i] = 0;
  
  c->accesstime = 0;
  
  return c;
  
}

Cache *  cache_free(Cache *c)
{
  if ( !c )           return NULL;
  if ( c->blocks   )  free(c->blocks);
  free(c);
  
  return NULL;
  
}

int cache_isvalid(Cache *c ) 
{
  if ( ! c )          return 0;
  if ( ! c->blocks )  return 0;
  if ( ! c->used   )  return 0;
  return 1;
}



void cache_print(Cache *c, int level)
{
  int i;
  
  if ( ! cache_isvalid(c) ) return;
  
  float hits_ratio = 100.0*(float)  c->hits  / (float) (c->hits+c->misses);
  float mean_time  = (float) c->accesstime / (float) (c->hits+c->misses);
  
  printf(" ----- current cache cache size %d strategy %d hits %d misses %d hits-ratio %3.3f %%  Mean acces time %lg   \n",
        c->nbrblocks, c->strategy, 
        c->hits,c->misses, hits_ratio, mean_time); 

  if ( level == 1 ) {
    for(i=0; i<c->nbrblocks; i++) {
      printf(" adr %p used %d \n",c->blocks[i],c->used[i]);
      //printf("%zu\n", sizeof(c->blocks[i]));
    }
  }
   
}


void    cache_reset(Cache *c)
{
  int i; 
  if ( ! cache_isvalid(c) ) return;
  
  c->hits   = 0;
  c->misses = 0;
  
  for(i=0; i<c->nbrblocks; i++) c->blocks[i] = NULL;
  for(i=0; i<c->nbrblocks; i++) c->used[i]   = 0;
  
  c->accesstime = 0;
  
  
}

static unsigned int cache_hashkey(char *key, int len, int siz){
    unsigned int i, h = 0;
    for(i=0;i<len;i++){
        h+=key[i];
        h+=(h<<10);
        h^=(h>>6);
    }
    h+=(h<<3);
    h^=(h>>11);
    h+=(h<<15);
    return(h%siz);

}

int my_cache_hashkey(void *adr, Cache* c){

    int code = (long long int)(adr)   % (c->nbrblocks+1) - 1;
    if(code <0) code = 0;
    return code;
}
void cache_fetchmemory(Cache *c, void *adr)
{
  int i;
  
  if ( ! cache_isvalid(c) ) return;
  
  
  
  if ( c->strategy == CACHE_DIRECT_MAPPING ) {
    //--- Direct mapping
    //int code = (long long int)(adr)  & (c->nbrblocks - 1);
    //int code = my_cache_hashkey(adr, c);
    int code = cache_hashkey((char*)adr,8,c->nbrblocks);
    //printf("%d \n", code);
    if (code >= c->nbrblocks )
        printf("out of index!!!");
    if ( c->blocks[code] == adr ) { 
      c->hits++;  c->used[code]   = 0; 
    }
    else  {
      c->blocks[code] = adr;
      c->used[code]   = 1;
      c->misses++;
    }
    
    c->accesstime += 1; 
    return ;
  }
  else if ( c->strategy == CACHE_FULLYASSOCIATIVE ) {
    int iMin   = INT_MAX;
    int iCode  = 0;
    for(i=0; i<c->nbrblocks; i++) {
      c->accesstime++;
      if ( c->blocks[i] == adr ) {
         c->used[i]++;
         c->hits++;
         return; 
      }
      if ( c->used[i] < iMin ) {
        iMin  = c->used[i];
        iCode = i; 
      }
    }
    
    c->blocks[iCode] = adr;
    c->used[iCode] = 1;
    c->misses++;
    return;
  }
  else if ( c->strategy == CACHE_2WAYASSOCIATIVE ) { 
    printf(" cache : strategy CACHE_2WAYASSOCIATIVE to do ! \n");
    exit(1); 
  }
  else if ( c->strategy == CACHE_4WAYASSOCIATIVE ) { 
    printf(" cache : strategy CACHE_4WAYASSOCIATIVE to do ! \n"); 
    exit(1); 
  }
  else  {
    printf(" cache : invalid cache strategy souuld be one of :  CACHE_DIRECT_MAPPING CACHE_FULLYASSOCIATIVE CACHE_2WAYASSOCIATIVE CACHE_4WAYASSOCIATIVE \n");
    exit(1);
  }
    
} 


static void data_randominit(int n, double *A) 
{
   int i;
   for(i=0; i<n; i++) A[i] = (float)rand()/(float)(RAND_MAX);
}
  
static void data_zeroinit(int n, double *A) 
{
   int i;
   for(i=0; i<n; i++) A[i] = 0.0;
}


void cache_dotprod_block(int n, Cache *c, int blocksize)
{
  int i;
  double dot = 0;
  
  double A[n];
  double B[n];
  int num_double_in_block = blocksize / 8;
  if ( ! cache_isvalid(c) ) return;
  
  data_randominit(n, A);
  data_randominit(n, B);
  int bloc_cached_index ;
  for(i=0; i<n; i++) {
    bloc_cached_index = i/num_double_in_block;
    cache_fetchmemory(c, (void *) &(A[bloc_cached_index]));
    cache_fetchmemory(c, (void *) &(B[bloc_cached_index]));
    dot = dot + A[i]*B[i];
    printf(" dot = %lg hits %d misses %d \n",dot,c->hits, c->misses);  
  }
  printf(" dot = %lg hits %d misses %d \n",dot,c->hits, c->misses);  
}  

void cache_dotprod(int n, Cache *c)
{
  int i;
  double dot = 0;
  
  double A[n];
  double B[n];
  if ( ! cache_isvalid(c) ) return;
  
  data_randominit(n, A);
  data_randominit(n, B);
  
  for(i=0; i<n; i++) {
    cache_fetchmemory(c, (void *) &(A[i]));
    cache_fetchmemory(c, (void *) &(B[i]));
    dot = dot + A[i]*B[i];
  }
  printf(" dot = %lg hits %d misses %d \n",dot,c->hits, c->misses);  
}  
  
void cache_matvec(int n, Cache *c)
{
  int i,j;
  double A[n][n];
  double B[n];
  double AB[n];
  
  if ( ! cache_isvalid(c) ) return;
 
  data_randominit(n*n, (double *) A);
  data_randominit(n, B);
  data_zeroinit(  n, AB);
  for(i=0; i<n; i++) {
    for(j=0; j<n; j++) {
      cache_fetchmemory(c, (void *) &(A[i][j]));
      cache_fetchmemory(c, (void *) &(B[j]));
      AB[i] = AB[i] + A[i][j]*B[j];
    }
  }
  printf("  hits %d misses %d \n",c->hits, c->misses);  
}
void cache_matvec_block(int n, Cache *c, int blocksize)
{
  int i,j;
  double A[n][n];
  double B[n];
  double AB[n];
  
  if ( ! cache_isvalid(c) ) return;
  int num_double_in_block = blocksize / 8;
  data_randominit(n*n, (double *) A);
  data_randominit(n, B);
  data_zeroinit(  n, AB);
  int bloc_cached_index;
  for(i=0; i<n; i++) {
    for(j=0; j<n; j++) {
      bloc_cached_index = j/num_double_in_block;
      cache_fetchmemory(c, (void *) &(A[i][bloc_cached_index]));
      cache_fetchmemory(c, (void *) &(B[bloc_cached_index]));
      AB[i] = AB[i] + A[i][j]*B[j];
    }
  }
  printf("  hits %d misses %d \n",c->hits, c->misses);  
}

void cache_matmat(int n, Cache *c)
{
  if ( ! cache_isvalid(c) ) return;
  
  printf("  cache_matmat :  not implemented \n");
  exit(1);
  
}


void cache_test()
{

  int size      = 256;
  int strategy  = CACHE_FULLYASSOCIATIVE;
  //int strategy  = CACHE_DIRECT_MAPPING;
  
  Cache * c = cache_init(size, strategy);
  
  cache_dotprod_block(1024 ,c,32);
  cache_print(c, 1);
  cache_reset(c);
  
  //cache_matvec ( 512 ,c);
  cache_matvec_block(512,c, 32);
  cache_print(c, 1);
  
  cache_reset(c); 
  
}



