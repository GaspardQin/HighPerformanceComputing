#ifndef PRIME_CPP
#define PRIME_CPP
#include "prim.h"
using namespace std;
double pi = std::acos(-1);

#define deg_to_rad_fac (M_PI / 180.0)
static inline int fls(int x)
{
int position;
int i;
if(0 != x)
{
for (i = (x >> 1), position = 0; i != 0; ++position)
           i >>= 1;
}
else
{
        position = -1;
} 
    return position+1;
}
static inline unsigned int roundup_pow_of_two(unsigned int x)
{
    return 1UL << fls(x - 1);
}

__device__ inline double sin_deg(double a){
        return (sin(a * deg_to_rad_fac));
}

__device__ inline double cos_deg(double a){
        return (cos(a * deg_to_rad_fac));
}


__device__ inline float getDistance(double lat1, double lon1, double lat2, double lon2){
  return  float(R_EARTH * acos(min(sin(lat1) * sin(lat2) 
             + cos_deg(lon1 - lon2) * cos(lat1)* cos(lat2),1.0)));
}
__global__ void compute_dist_functor(float * lat, float* lon, float * min_dist,const int compare_index, const int maxN)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < maxN){
    min_dist[i] = getDistance(lat[i], lon[i], lat[compare_index],lon[compare_index]);
  }
};

__global__ void init_dist_functor(float * lat, float* lon, float * min_dist,int* parent, float* distance_total_dev, const int compare_index, const int maxN)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  
  if(i < maxN){
    if(i == compare_index)  min_dist[i] = -1.0;
    else min_dist[i] = getDistance(lat[i], lon[i], lat[compare_index],lon[compare_index]);
    
    parent[i] = compare_index;

  }
  if(i == 0){
    distance_total_dev[0] = 0.0;
  }
};

__global__ void  update_min_dist_functor(float* min_dist, float* lat, float*  lon, int* parent,int* new_in_index, const int maxN)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < maxN)
  {  
    if(min_dist[i] < 0)
      return;
    else{
      float temp = getDistance(lat[i], lon[i],lat[(*new_in_index)], lon[(*new_in_index)]);
      if(temp < min_dist[i]){
        parent[i] = (*new_in_index);
      }
    }
  }
};


__global__ void  set_all_parent_functor(int * parent,int parent_default, const int maxN)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < maxN){  
      parent[i] = parent_default;    
  }
};


__global__ void reduce_find_min_block(float *min_dist, int * min_min_index_block, float*min_min_dist_block, int nbVilles) 
{
  __shared__ float min[256];
  __shared__ int min_index[256];
  unsigned int tid = threadIdx.x;
  unsigned int i = blockIdx.x *blockDim.x + threadIdx.x;
  int nTotalThreads = blockDim.x;
  // each thread loads one element from global to shared mem
  //unsigned int tid = threadIdx.x;
  if(i < nbVilles){
    min[tid] = min_dist[i];
    min_index[tid] = i,
    __syncthreads();


    while(nTotalThreads > 1)
    {
      int halfPoint = (nTotalThreads >> 1); // divide by two
      if (tid < halfPoint)
      {
        int thread2 = tid + halfPoint;
        if(min[thread2] >= 0 && (min[thread2] < min[tid] || min[tid] < 0)){
          min[tid] = min[thread2];
          min_index[tid] = thread2;
        }
      }
      __syncthreads();
      nTotalThreads = halfPoint;
    }
    if(tid == 0){
    
      min_min_index_block[blockIdx.x] = min_index[0];
      min_min_dist_block[blockIdx.x] = min[0];
      //min_dist[(*min_min_index)] = -1.0;
      //*distance_total_dev += (*min_min_dist);
    }
  }
}
__global__ void reduce_find_min(float* min_dist,int * min_min_index_block, float*min_min_dist_block, float* distance_total_dev, int* min_min_index, float* min_min_dist, int block_size) 
{
  __shared__ float min[512];
  __shared__ int min_index[512];

  unsigned int i = threadIdx.x;
  int nTotalThreads = blockDim.x;
  // each thread loads one element from global to shared mem
  //unsigned int tid = threadIdx.x;
  if(i < block_size){
    min[i] = min_min_dist_block[i];
    min_index[i] = min_min_index_block[i],
    __syncthreads();


    while(nTotalThreads > 1)
    {
      int halfPoint = (nTotalThreads >> 1); // divide by two
      if (i < halfPoint)
      {
        int thread2 = i + halfPoint;
        if(min[thread2] >= 0 && (min[thread2] < min[i] || min[i] < 0)){
          min[i] = min[thread2];
          min_index[i] = thread2;
        }
      }
      __syncthreads();
      nTotalThreads = halfPoint;
    }
    if(i == 0){
    
      *min_min_index = min_index[0];
      *min_min_dist = min[0];
      min_dist[(*min_min_index)] = -1.0;
      *distance_total_dev += (*min_min_dist);
    }
  }
}


void prim(float *lat_host, float* lon_host, int* parent_host,
          const int nbVilles, float *distance_total)
{
  //int threadsPerBlock = 256;
  //int blocksPerGrid =(nbVilles + threadsPerBlock - 1) / threadsPerBlock;

  dim3 threadsPerBlock(128); 
  const int block_size =  (nbVilles + threadsPerBlock.x - 1) / threadsPerBlock.x;
  dim3 blocksPerGrid(block_size);  

  // Allocate the device input vector 
  float *lat_dev = NULL;
  float *lon_dev = NULL;
  int *parent_dev = NULL;
  float *min_dist_dev = NULL;
  float  *distance_total_dev = NULL;
  int* min_min_index_dev = NULL;
  float* min_min_dist_dev = NULL;
  int* min_min_index_dev_block = NULL;
  float* min_min_dist_dev_block = NULL;
  const int memory_size_float =  nbVilles * sizeof(float);
  const int memory_size_int =  nbVilles * sizeof(float);
  cudaMalloc((void **)&distance_total_dev, sizeof(float));
  cudaMalloc((void **)&min_min_index_dev, sizeof(int));
  cudaMalloc((void **)&min_min_dist_dev, sizeof(float));
  cudaMalloc((void **)&min_min_index_dev_block, block_size*sizeof(int));
  cudaMalloc((void **)&min_min_dist_dev_block, block_size*sizeof(float));


  cudaError_t err_lat = cudaMalloc((void **)&lat_dev, memory_size_float);
  cudaError_t err_lon = cudaMalloc((void **)&lon_dev, memory_size_float);
  cudaError_t err_min_dist = cudaMalloc((void **)&min_dist_dev, memory_size_float);
  cudaError_t err_parent = cudaMalloc((void **)&parent_dev, memory_size_int);
  if (err_lat != cudaSuccess )
  {
      fprintf(stderr, "Failed to allocate device vector lat_dev(error code %s)!\n", cudaGetErrorString(err_lat));
      exit(EXIT_FAILURE);
  }
  if (err_lon != cudaSuccess )
  {
      fprintf(stderr, "Failed to allocate device vector lon_dev(error code %s)!\n", cudaGetErrorString(err_lon));
      exit(EXIT_FAILURE);
  }
  if (err_parent != cudaSuccess )
  {
      fprintf(stderr, "Failed to allocate device vector parent_dev(error code %s)!\n", cudaGetErrorString(err_parent));
      exit(EXIT_FAILURE);
  }
  if (err_min_dist != cudaSuccess )
  {
      fprintf(stderr, "Failed to allocate device vector min_dist_dev(error code %s)!\n", cudaGetErrorString(err_min_dist));
      exit(EXIT_FAILURE);
  }

  // copy to GPU memory
  err_lat = cudaMemcpy(lat_dev, lat_host, memory_size_float, cudaMemcpyHostToDevice);
  err_lon = cudaMemcpy(lon_dev, lon_host, memory_size_float, cudaMemcpyHostToDevice);
  err_parent = cudaMemcpy(parent_dev, parent_host, memory_size_int, cudaMemcpyHostToDevice);
  if (err_lat != cudaSuccess)
  {
      fprintf(stderr, "Failed to copy vector lat from host to device (error code %s)!\n", cudaGetErrorString(err_lat));
      exit(EXIT_FAILURE);
  }
  if (err_lon != cudaSuccess)
  {
      fprintf(stderr, "Failed to copy vector lon from host to device (error code %s)!\n", cudaGetErrorString(err_lon));
      exit(EXIT_FAILURE);
  }
  if (err_parent != cudaSuccess)
  {
      fprintf(stderr, "Failed to copy vector parent from host to device (error code %s)!\n", cudaGetErrorString(err_parent));
      exit(EXIT_FAILURE);
  }
  


  // init Prim
  /* 
	for(i = 1; i < nbVilles; i++)
	{
		min_dist[i] = distance[0][i];
		parent[i] = 0;
	}
  */
  init_dist_functor<<<blocksPerGrid, threadsPerBlock>>>(lat_dev, lon_dev, min_dist_dev, parent_dev,distance_total_dev, 0, nbVilles);

  cudaDeviceSynchronize();
  //cout<< "init: ==================="<<endl;
  //float * min_dist_host = (float*)malloc(nbVilles * sizeof(float));
  //cudaMemcpy(min_dist_host, min_dist_dev, memory_size_float, cudaMemcpyDeviceToHost);
  //cudaMemcpy(parent_host, parent_dev,memory_size_int, cudaMemcpyDeviceToHost);
  //cudaMemcpy(distance_total, distance_total_dev, sizeof(float), cudaMemcpyDeviceToHost);
  //debug_print(min_dist_host, parent_host,*distance_total, nbVilles);


	cudaError_t err = cudaGetLastError();
  if (err != cudaSuccess )
  {
      fprintf(stderr, "Failed to run kernel init_dist_functor(error code %s)!\n", cudaGetErrorString(err));
      exit(EXIT_FAILURE);
  }
  
  //iteration of Prime
  int k;
	for(k = 0; k < nbVilles -1 ; k++)
	{
    /*
		// find the minimal min_dist outstide of S
		min_min_dist = FLT_MAX;

		for(i = 0; i < nbVilles; i++)
		{
      __assume_aligned(min_dist, VEC_ALIGN);
				if(min_dist[i] >0 && min_min_dist > min_dist[i])
				{
					min_min_dist = min_dist[i];
					min_min_dist_index = i;
				}
		}
  */
//reduce_find_min_block(float *min_dist, int * min_min_index_block, float*min_min_dist_block, int nbVilles, int array_size) 
//reduce_find_min(float* min_dist,int * min_min_index_block, float*min_min_dist_block, float* distance_total_dev, int* min_min_index, float* min_min_dist) 
    //cout <<"roudup : "<< roundup_pow_of_two(nbVilles) << endl;
    reduce_find_min_block<<<blocksPerGrid, threadsPerBlock>>>(min_dist_dev, min_min_index_dev_block, min_min_dist_dev_block, nbVilles);
    cudaDeviceSynchronize();
    if (err != cudaSuccess )
    {
        fprintf(stderr, "Failed to run kernel reduce_find_min_block(error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    reduce_find_min<<<1,  blocksPerGrid>>>(min_dist_dev, min_min_index_dev_block, min_min_dist_dev_block,distance_total_dev, min_min_index_dev,min_min_dist_dev,block_size);
    err = cudaGetLastError();
    if (err != cudaSuccess )
    {
        fprintf(stderr, "Failed to run kernel reduce_find_min(error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
  

    //float min_min_dist_host;
    //float min_min_index_host;
    //cudaMemcpy(&min_min_index_host, min_min_index_dev, sizeof(int), cudaMemcpyDeviceToHost);
    //cudaMemcpy(&min_min_dist_host, min_min_dist_dev, sizeof(float), cudaMemcpyDeviceToHost);
    //cudaMemcpy(distance_total, distance_total_dev, sizeof(float), cudaMemcpyDeviceToHost);
    //cout << "min_dist: "<< min_min_dist_host << "index: "<< min_min_index_host << " distance_total: "<< *distance_total<<endl;

    //update the min_dist
    /*
    int dist_temp;

    float* distance_i_ptr =  distance[min_min_dist_index];
		for(j = 0; j < nbVilles; j++)
		{
      __assume_aligned(distance_i_ptr, VEC_ALIGN);
      dist_temp = distance_i_ptr[j];

			if(min_dist[j] > dist_temp)
			{
				min_dist[j] = dist_temp;
				parent[j] = min_min_dist_index;
			}
		}
    */
    update_min_dist_functor<<<blocksPerGrid, threadsPerBlock>>>(min_dist_dev, lat_dev, lon_dev, parent_dev ,min_min_index_dev, nbVilles);
    cudaDeviceSynchronize();
    err = cudaGetLastError();
    if (err != cudaSuccess )
    {
        fprintf(stderr, "Failed to run kernel update_min_dist_functor(error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
  }
  err_parent = cudaMemcpy(parent_host, parent_dev, memory_size_int, cudaMemcpyDeviceToHost);
  cudaMemcpy(distance_total, distance_total_dev, sizeof(float), cudaMemcpyDeviceToHost);
  cudaFree(distance_total_dev);
  cudaFree(min_min_index_dev);
  cudaFree(min_min_dist_dev);
  cudaFree(lat_dev);
  cudaFree(lon_dev);
  cudaFree(min_dist_dev);
  cudaFree(parent_dev);
  //free(min_dist_host);
}


#endif
