#ifndef PRIME_CPP
#define PRIME_CPP
#include "prim.h"
using namespace std;
float pi = std::acos(-1);

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

__device__ __host__ inline double sin_deg(double a){
        return (sin(a * deg_to_rad_fac));
}

__device__ __host__ inline double cos_deg(double a){
        return (cos(a * deg_to_rad_fac));
}


__device__ inline float getDistance(double sin_lat1,double cos_lat1, double lon1, double sin_lat2, double cos_lat2, double lon2){
  return  float(R_EARTH * acos(fmin(sin_lat1 * sin_lat2 
             + cos_deg(lon1 - lon2) * cos_lat1* cos_lat2,1.0)));
}

__global__ void compute_sincoslat_functor(float * lat, double * sin_lat, double* cos_lat,const int maxN)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < maxN){
    double temp = lat[i] * deg_to_rad_fac;
    sincos(temp, &(sin_lat[i]), &(cos_lat[i]) );
  }
};




__global__ void init_dist_functor(double * sin_lat, double* cos_lat, float* lon, float * min_dist,int* parent, float* distance_total_dev, const int compare_index, const int maxN)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  
  if(i < maxN){
    if(i == compare_index)  min_dist[i] = FLT_MAX;
    else min_dist[i] = getDistance(sin_lat[i], cos_lat[i], lon[i], sin_lat[compare_index], cos_lat[compare_index],lon[compare_index]);
    
    parent[i] = compare_index;

  }
  if(i == 0){
    distance_total_dev[0] = 0.0;
  }
};

__global__ void  update_min_dist_functor(float* min_dist, double* sin_lat, double* cos_lat, float*  lon, int* parent,int* new_in_index, const int maxN)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < maxN)
  {  
    if(min_dist[i] >= FLT_MAX){
      return;
    }
    else{
      float temp = getDistance(sin_lat[i], cos_lat[i], lon[i],sin_lat[(*new_in_index)],cos_lat[(*new_in_index)], lon[(*new_in_index)]);
      if(temp < min_dist[i]){
        
        min_dist[i] = temp;
        
        parent[i] = (*new_in_index);
      }
    }
  }
};



__global__ void reduce_find_min_block(float *min_dist, int * min_min_index_block, float*min_min_dist_block, int nbVilles) 
{
 
  unsigned int tid = threadIdx.x;
  unsigned int i = blockIdx.x *blockDim.x + threadIdx.x;
  int nTotalThreads = blockDim.x;
  // each thread loads one element from global to shared mem
  //unsigned int tid = threadIdx.x;
  __shared__ float min[256];
  __shared__ int min_index[256];
  if(i<nbVilles){
    min[tid] = min_dist[i];
    min_index[tid] = i;
  }
  else{
    min[tid] = FLT_MAX;
    min_index[tid] = -1;
  }
  __syncthreads();


  while(nTotalThreads > 1)
  {
    int quaterPoint = (nTotalThreads >> 2); // divide by two
    if (tid < quaterPoint)
    {
      int thread2 = tid + quaterPoint;
      if(min[thread2] < min[tid] ){
        min[tid] = min[thread2];
        min_index[tid] = min_index[thread2];
      }
      thread2 = tid + 2* quaterPoint;
      if(min[thread2] < min[tid] ){
        min[tid] = min[thread2];
        min_index[tid] = min_index[thread2];
      }
      thread2 = tid + 3* quaterPoint;
      if(min[thread2] < min[tid] ){
        min[tid] = min[thread2];
        min_index[tid] = min_index[thread2];
      }
    }
    __syncthreads();
    nTotalThreads = quaterPoint;
  }

  if(tid == 0){
  
    min_min_index_block[blockIdx.x] = min_index[0];
    min_min_dist_block[blockIdx.x] = min[0];
    //min_dist[(*min_min_index)] = -1.0;
    //*distance_total_dev += (*min_min_dist);
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
    min_index[i] = min_min_index_block[i];
  }
  else{
    min[i] = FLT_MAX;
    min_index[i] = -1;
  }
    __syncthreads();


    while(nTotalThreads > 1)
    {
      int quaterPoint = (nTotalThreads >> 2); // divide by two
      if (i < quaterPoint)
      {
        int thread2 = i + quaterPoint;
        if(min[thread2] < min[i]){
          min[i] = min[thread2];
          min_index[i] = min_index[thread2];
        }
        thread2 = i + 2 * quaterPoint;
        if(min[thread2] < min[i]){
          min[i] = min[thread2];
          min_index[i] = min_index[thread2];
        }
        thread2 = i + 3 * quaterPoint;
        if(min[thread2] < min[i]){
          min[i] = min[thread2];
          min_index[i] = min_index[thread2];
        }
      }
      __syncthreads();
      nTotalThreads = quaterPoint;
    }
    if(i == 0){
    
      *min_min_index = min_index[0];
      *min_min_dist = min[0];
      min_dist[(*min_min_index)] = FLT_MAX;
      *distance_total_dev += (*min_min_dist);
    }
  
}
void prim(float *lat_host, float* lon_host, int* parent_host,
          const int nbVilles, float *distance_total)
{
  //int threadsPerBlock = 256;
  //int blocksPerGrid =(nbVilles + threadsPerBlock - 1) / threadsPerBlock;

  dim3 threadsPerBlock(256); 
  const int block_size_ =  (nbVilles + threadsPerBlock.x - 1) / threadsPerBlock.x;
  dim3 blocksPerGrid(block_size_);  
  
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
  double* sin_lat_dev = NULL;
  double* cos_lat_dev = NULL;


  const int memory_size_float =  nbVilles * sizeof(float);
  const int memory_size_int =  nbVilles * sizeof(int);
  cudaMalloc((void **)&distance_total_dev, sizeof(float));
  cudaMalloc((void **)&min_min_index_dev, sizeof(int));
  cudaMalloc((void **)&min_min_dist_dev, sizeof(float));
  cudaMalloc((void **)&min_min_index_dev_block, block_size_*sizeof(int));
  cudaMalloc((void **)&min_min_dist_dev_block, block_size_*sizeof(float));
  cudaMalloc((void **)&sin_lat_dev, nbVilles*sizeof(double));
  cudaMalloc((void **)&cos_lat_dev, nbVilles*sizeof(double));


  cudaError_t err_lat = cudaMalloc((void **)&lat_dev, memory_size_float);
  cudaError_t err_lon = cudaMalloc((void **)&lon_dev, memory_size_float);
  cudaError_t err_min_dist = cudaMalloc((void **)&min_dist_dev, memory_size_float);
  cudaError_t err_parent = cudaMalloc((void **)&parent_dev, memory_size_int);

  //float *debug_min_dist_dev = NULL;
  //cudaMalloc((void **)&debug_min_dist_dev, nbVilles*sizeof(float));
  //float *debug_min_dist_host = (float*) malloc(nbVilles*sizeof(float));
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
  compute_sincoslat_functor<<<blocksPerGrid, threadsPerBlock>>>(lat_dev, sin_lat_dev, cos_lat_dev,nbVilles);
  init_dist_functor<<<blocksPerGrid, threadsPerBlock>>>(sin_lat_dev,cos_lat_dev, lon_dev, min_dist_dev, parent_dev,distance_total_dev, 0, nbVilles);

  cudaDeviceSynchronize();
  ////cout<< "init: ==================="<<endl;
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

  //for debug
  //float * min_min_dist_host_block = (float*)malloc(block_size_ * sizeof(float));
  //int * min_min_index_host_block = (int*)malloc(block_size_ * sizeof(int));
  //float * min_dist_host = (float*)malloc(nbVilles * sizeof(float));
  //cudaMemcpy(min_dist_host, min_dist_dev, nbVilles * sizeof(float), cudaMemcpyDeviceToHost);
  //validation
  //float* debug_min_dist_validation_host = (float*)malloc(memory_size_float);
  //for(int i = 0 ; i<nbVilles; i++)
  // debug_min_dist_validation_host[i] = R_EARTH * std::acos( std::min(sin_deg(lat_host[i]) * sin_deg(lat_host[0])  + cos_deg(lon_host[i]-lon_host[0]) * cos_deg(lat_host[i])* cos_deg(lat_host[0]), 1.0));

  //debug_print(debug_min_dist_validation_host, min_dist_host, nbVilles);
  //cout << "-------------------"<<endl;
  

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
    //cudaMemcpy(min_dist_host, min_dist_dev, nbVilles * sizeof(float), cudaMemcpyDeviceToHost);
    
    //debug_print(min_dist_host, nbVilles );
    //cout<< "<<<<<<<<<<" <<endl;
    reduce_find_min_block<<<blocksPerGrid,threadsPerBlock >>>(min_dist_dev, min_min_index_dev_block, min_min_dist_dev_block, nbVilles);
    cudaDeviceSynchronize();
    if (err != cudaSuccess )
    {
        fprintf(stderr, "Failed to run kernel reduce_find_min_block(error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }


    
    //cudaMemcpy(min_min_index_host_block, min_min_index_dev_block, block_size_ * sizeof(int), cudaMemcpyDeviceToHost);
    //cudaMemcpy(min_min_dist_host_block, min_min_dist_dev_block, block_size_ * sizeof(float), cudaMemcpyDeviceToHost);
    
    //debug_print(min_min_index_host_block,min_min_dist_host_block, block_size_ );
    //cout << "============"<<endl;
    //cout << "std min dist:" << *(min_element(min_dist_host, min_dist_host+nbVilles))<<endl;
    //cout << "block_size: "<<block_size_ << " nbVilles: "<< nbVilles <<endl;
    reduce_find_min<<<1,  roundup_pow_of_two(block_size_)>>>(min_dist_dev, min_min_index_dev_block, min_min_dist_dev_block,distance_total_dev, min_min_index_dev,min_min_dist_dev,block_size_);
    err = cudaGetLastError();
    if (err != cudaSuccess )
    {
        fprintf(stderr, "Failed to run kernel reduce_find_min(error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    //float min_min_dist_host;
    //int min_min_index_host;
    //cudaMemcpy(min_dist_host, min_dist_dev, memory_size_float, cudaMemcpyDeviceToHost);
    //cudaMemcpy(parent_host, parent_dev,memory_size_int, cudaMemcpyDeviceToHost);
    //cudaMemcpy(&min_min_index_host, min_min_index_dev,sizeof(int), cudaMemcpyDeviceToHost);
    //cudaMemcpy(distance_total, distance_total_dev, sizeof(float), cudaMemcpyDeviceToHost);
    //cudaMemcpy(&min_min_dist_host, min_min_dist_dev, sizeof(float), cudaMemcpyDeviceToHost);
    //debug_print(min_dist_host, parent_host,*distance_total, min_min_index_host, k, nbVilles);
    //cout<<"total: "<< nbVilles << " min_min_index: "<<min_min_index_host<< " min_min_dist: "<< min_min_dist_host<< " dist_toal: "<<*distance_total<<endl;
    
    //debug_print(min_dist_host, parent_host,*distance_total, min_min_index_host, k, nbVilles);

    //float min_min_dist_host;
    //float min_min_index_host;
    //cudaMemcpy(&min_min_index_host, min_min_index_dev, sizeof(int), cudaMemcpyDeviceToHost);
    //cudaMemcpy(&min_min_dist_host, min_min_dist_dev, sizeof(float), cudaMemcpyDeviceToHost);
    //cudaMemcpy(distance_total, distance_total_dev, sizeof(float), cudaMemcpyDeviceToHost);
    ////cout << "min_dist: "<< min_min_dist_host << "index: "<< min_min_index_host << " distance_total: "<< *distance_total<<endl;

    update_min_dist_functor<<<blocksPerGrid, threadsPerBlock>>>(min_dist_dev, sin_lat_dev, cos_lat_dev, lon_dev, parent_dev ,min_min_index_dev, nbVilles);
    cudaDeviceSynchronize();
    err = cudaGetLastError();
    if (err != cudaSuccess )
    {
        fprintf(stderr, "Failed to run kernel update_min_dist_functor(error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    //cudaMemcpy(debug_min_dist_host, debug_min_dist_dev, memory_size_float, cudaMemcpyDeviceToHost);
    //cudaMemcpy(min_dist_host, min_dist_dev, memory_size_float, cudaMemcpyDeviceToHost);
    //cudaMemcpy(parent_host, parent_dev,memory_size_int, cudaMemcpyDeviceToHost);
    //debug_print(debug_min_dist_host, min_dist_host, nbVilles);

        #ifdef SHOW_EVERY_STEPS
        // check and create folder "steps"
        int iRet = access("./steps",0);
        if (iRet != 0)
                iRet = mkdir("./steps",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        // create files steps files to store data
        string save_name_base = "./steps/resuGraphe_";
        string save_name = save_name_base + to_string(k)+".dat";
        ////cout<<"steps: "<< 0<<endl;
        ofstream fileOut(save_name);
        for(int i =0; i < nbVilles; i++)
        {
        fileOut << parent_host[i] << " "<< i <<" "<<min_dist_host[i] << " " << int(min_dist_host[i] >= (FLT_MAX-10)) <<"\n";
        }
        fileOut.close();
        #endif // SHOW_EVERY_STEPS




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
  //cudaFree(debug_min_dist_dev);
  //free(min_min_dist_host_block);
  //free(min_min_index_host_block);
  //free(min_dist_host);
  //free(debug_min_dist_host);
  //free(debug_min_dist_validation_host);
}


#endif
