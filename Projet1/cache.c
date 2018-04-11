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


  c->lastUse   = malloc(sizeof(int) * size );
  for(i=0; i<c->nbrblocks; i++) c->lastUse[i] = 0;

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
  double mean_time  = (double) c->accesstime / (double) (c->hits+c->misses);

  printf(" ----- current cache cache size %d strategy %d hits %d misses %d hits-ratio %3.3f %%  Mean acces time %f   \n",
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
  for(i=0; i<c->nbrblocks; i++) c->lastUse[i]   = 0;

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
         c->lastUse[i] = c->accesstime;
         return;
      }
      if ( c->lastUse[i] < iMin ) {
        iMin  = c->lastUse[i];
        iCode = i;
      }
    }

    c->blocks[iCode] = adr;
    c->used[iCode] = 1;
    c->lastUse[iCode] = c->accesstime;
    c->misses++;
    return;
  }
  else if ( c->strategy == CACHE_2WAYASSOCIATIVE ) {
    int iMinTime = INT_MAX;
    int code = cache_hashkey((char*)adr,8,c->nbrblocks/2);
    int iCode  = 0;
    for(i=0;i<2;i++){
      c->accesstime ++;
      int asso_index = i*(int)((c->nbrblocks)/2)+code;
      if(c->blocks[asso_index] == adr){
        c->used[asso_index] ++;
        c->hits++;
        c->lastUse[asso_index] = c->accesstime;
        return;
      }
      if(c->lastUse[asso_index]<iMinTime){
        iMinTime = c->lastUse[asso_index];
        iCode = asso_index;
      }
    }
    c->blocks[iCode] = adr;
    c->used[iCode] = 1;
    c->lastUse[iCode] = c->accesstime;
    c->misses++;
    return;
  }
  else if ( c->strategy == CACHE_4WAYASSOCIATIVE ) {
    int iMinTime = INT_MAX;
    int code = cache_hashkey((char*)adr,8,c->nbrblocks/4);
    int iCode  = 0;
    for(i=0;i<4;i++){
      c->accesstime ++;
      int asso_index = i*(int)((c->nbrblocks)/4)+code;
      if(c->blocks[asso_index] == adr){
        c->used[asso_index] ++;
        c->hits++;
        c->lastUse[asso_index] = c->accesstime;
        return;
      }
      if(c->lastUse[asso_index]<iMinTime){
        iMinTime = c->lastUse[asso_index];
        iCode = asso_index;
      }
    }
    c->blocks[iCode] = adr;
    c->used[iCode] = 1;
    c->lastUse[iCode] = c->accesstime;
    c->misses++;
    return;
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
    //printf(" dot = %lg hits %d misses %d \n",dot,c->hits, c->misses);
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

void cache_matmat_block(int n, Cache *c, int blocksize, int mat_block_coeffi)
{
    //mat_block_coeffi means the matrix block's width is (mat_block_coeffi * num_double_in_block)
  if ( ! cache_isvalid(c) ) return;
  int i_A,j_A,j_B;
  int i_block_A, j_block_A, j_block_B;
  double A[n][n];
  double B[n][n];
  double AB[n][n];

  int num_double_in_block = blocksize / 8;
  data_randominit(n*n, (double *) A);
  data_randominit(n*n, (double *) B);
  data_zeroinit(n*n, (double *)AB);
  int mat_block_width = mat_block_coeffi *  num_double_in_block;
  for(i_block_A=0; i_block_A<n; i_block_A+=mat_block_width) { //num_double_in_block lines for each iterate
    for(j_block_A = 0; j_block_A < n; j_block_A += mat_block_width ){

      //in block
      for(j_B = 0; j_B<n; j_B ++){

        for(i_A = i_block_A; i_A<mat_block_width + i_block_A;i_A++){
          j_block_B = j_B % mat_block_width;
          int temp = 0;
          for(j_A =j_block_A; j_A<mat_block_width + j_block_A; j_A++) {
            cache_fetchmemory(c, (void *) &(A[i_A][(int)(j_A/num_double_in_block)]));

            cache_fetchmemory(c, (void *) &(B[j_A][(int)(j_B/num_double_in_block)]));

            temp += A[i_A][j_A] * B[j_A][j_B];
          }
          cache_fetchmemory(c, (void *) &(AB[i_A][(int)(j_B/num_double_in_block)])); //also compte the access of AB matrix
          AB[i_A][j_B] += temp;

        }

      }
    }
    //printf("write AB line %d\n", i_block_A);
  }
  printf("Matrix prod Marix  hits %d misses %d \n",c->hits, c->misses);

  //printf("  cache_matmat :  not implemented \n");
  //exit(1);

}


void cache_test(int nbr_block, int strategy, int block_size, int vec_size, int mat_bloc_prop)
{

  //int size      = nbr_block; //128
  //int strategy  = CACHE_FULLYASSOCIATIVE;
  //int strategy  = CACHE_DIRECT_MAPPING;
  //int strategy  = CACHE_2WAYASSOCIATIVE;
  //int strategy = CACHE_4WAYASSOCIATIVE;
  Cache * c = cache_init(nbr_block, strategy);


  //cache_dotprod(1024 ,c);
  cache_dotprod_block(vec_size ,c,block_size);
  cache_print(c, 0);
  cache_reset(c);

  //cache_matvec ( 512 ,c);
  cache_matvec_block(vec_size,c, block_size);
  cache_print(c, 0);
  cache_reset(c);

  //mat matrix

  cache_matmat_block(vec_size, c, block_size,mat_bloc_prop);
  cache_print(c, 0);
  cache_reset(c);

}
