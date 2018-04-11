#include <cache.h>

int main(int argc, char *argv[])
{
    int nbr_block = atoi(argv[1]);
    int strategy = atoi(argv[2]);
    int block_size = atoi(argv[3]);
    int vec_size = atoi(argv[4]);
    int mat_bloc_prop = atoi(argv[5]);
    printf("nbr_block: %d, strategy: %d,  block_size: %d, vec_size: %d, mat_block_prop: %d\n",nbr_block, strategy, block_size, vec_size, mat_bloc_prop );
    cache_test( nbr_block,  strategy,  block_size,  vec_size,  mat_bloc_prop);
    return 0;
}
