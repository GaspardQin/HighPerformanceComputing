#ifndef PRIME_CPP
#define PRIME_CPP
#include "prim.h"
using namespace std;
const double pi = std::acos(-1);
const double deg_to_rad_fac = pi / 180.0;
inline double sin_deg(double a){
        return (thrust::sin(a * deg_to_rad_fac));
}
inline double cos_deg(double a){
        return (thrust::cos(a * deg_to_rad_fac));
}

void computeDistance(float* &villesLon, float* &villesLat, const int nbVilles, float** &distance)
{
	int i, j;

	// inital
	//distance = new float* [nbVilles];
  distance = (float**)_mm_malloc(nbVilles * sizeof(float*), VEC_ALIGN);
  //float* sin_lat = new float[nbVilles];
  float* sin_lat = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
  //float* cos_lat = new float[nbVilles];
  float* cos_lat = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
  //#pragma omp parallel for simd num_threads(4) schedule(dynamic,4)

	for(i = 0; i < nbVilles; i++)
	{
    __assume_aligned(distance, VEC_ALIGN);
		distance[i] = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
    //distance[i] = (float*) malloc(nbVilles * sizeof(float));
	}
  #pragma ivdep
  for(i = 0; i < nbVilles; i++){
    __assume_aligned(villesLat, VEC_ALIGN);
    sin_lat[i] = sin_deg(villesLat[i]);
    cos_lat[i] = cos_deg(villesLat[i]);
  }
	// compute distance
  int jj,ii; int jj_max, ii_max;
  float* distance_ii_ptr;
  float distance_temp_block;
  #pragma omp parallel for num_threads(4) schedule(dynamic,4)
	for(i = 0; i < nbVilles; i+= BLOCK_SIZE)
	{
		for( j = 0; j <= i; j += BLOCK_SIZE){
      ii_max = min(nbVilles, i + BLOCK_SIZE);

      for(ii = i; ii < ii_max; ii++){
        __assume_aligned(distance, VEC_ALIGN);
        jj_max = min(ii+1, j + BLOCK_SIZE);
        distance_ii_ptr = distance[ii];
        #pragma ivdep
        for(jj = j; jj < jj_max; jj++ ){
          __assume_aligned(sin_lat, VEC_ALIGN);
          __assume_aligned(cos_lat, VEC_ALIGN);
          __assume_aligned(distance_ii_ptr, VEC_ALIGN);
          __assume_aligned(villesLon, VEC_ALIGN);
          //cout<<"ii: "<<ii <<" jj:"<<jj<<endl;
              //distance_temp_block[jj-j] =  R * acos( sin_lat[i] * sin_lat[jj]  + cos_deg(villesLon[i]-villesLon[jj]) * cos_lat[i]* cos_lat[jj]);
          distance_temp_block =  R * acosf( sin_lat[ii] * sin_lat[jj]  + cos_deg(villesLon[ii]-villesLon[jj]) * cos_lat[ii]* cos_lat[jj]);
          //cout<<"ii: "<<ii <<" jj:"<<jj : <<endl;

          distance_ii_ptr[jj] = distance_temp_block;
          distance[jj][ii] = distance_temp_block;
        }
      }
    }
	}

  _mm_free(cos_lat);
  _mm_free(sin_lat);
}

inline double getDistance(double lat1, double lon1, double lat1, double lat2){
  return R * thrust::acos( thrust::min(thrust::sin(lat1) * thrust::sin(lat2) 
             + cos_deg(lon1 - lon2) * thrust::cos(lat1)* thrust::cos(lat2), 1.0));
}


struct compute_dist_functor
{
    POS * pos_ptr;
    int compare_index;
    compute_dist_functor(POS * pos_ptr_, int compare_index_): pos_ptr(pos_ptr_), compare_index(compare_index_){};
    __host__ __device__
        void operator()(const int& a) const{ 
          (pos_ptr+a)->min_dist = getDistance((pos_ptr+a)->lat, (pos_ptr+a)->lon, (pos_ptr+compare_index)->lat,(pos_ptr+compare_index)->lon);
        }
};


struct update_min_dist_functor
{
  POS * pos_ptr;
  int new_in_index;
  double temp;
  update_min_dist_functor(POS * pos_ptr_, int new_in_index_): pos_ptr(pos_ptr_), new_in_index(new_in_index_){};
  
  __host__ __device__
  void operator()(const int& a)
  {
    if((pos_ptr+a)->is_in == true)
      return;
    else{
      temp = getDistance((pos_ptr+a)->lat, (pos_ptr+a)->lon, (pos_ptr+new_in_index)->lat,(pos_ptr+new_in_index)->lon);
      if(temp < (pos_ptr+a)->min_dist){
        (pos_ptr+a)->parent = new_in_index;
      }
    }
  }
};


struct set_all_parent_functor
{
    int parent_default;
    __host__ __device__
    set_all_parent_functor(int a):(parent_default(a)){};
    void operator()(const POS& A){ 
      A.parent = parent_default;    
    }
};
struct compute_min_dist_functor{
    POS * pos_ptr;
    int compare_index;
    compute_dist_functor(POS * pos_ptr_, int compare_index_): pos_ptr(pos_ptr_), compare_index(compare_index_){};
    
    __host__ __device__
        void operator()(const int& a){ 
            (pos_ptr + compare_index)->min_dist =  getDistance((pos_ptr+a)->lat, (pos_ptr+a)->lon, (pos_ptr+compare_index)->lat,(pos_ptr+compare_index)->lon);
        }
};


struct compare_min_dist_functor
{
  __host__ __device__
  bool operator()(POS a, POS b)
  {
    return a.min_dist + is_in * FLT_MAX < b.min_dist + is_in * FLT_MAX;
  }
};

void prim(thrust::host_vector<POS> &villesPosVecHost,const int nbVilles, double &distance_total)
{
  // copy to GPU memory
  thrust::device_vector<POS> villesPosVecDev(nbVilles);
  thrust::copy(villesPosVecHost.begin(), villesPosVecHost.end(), villesPosVecDev.begin());
  thrust::device_vector<POS> indexSequenceDev(nbVilles);
  thrust::sequence(indexSequenceDev.begin(), indexSequenceDev.end());
	// init Prime
  /* 
	for(i = 1; i < nbVilles; i++)
	{
		min_dist[i] = distance[0][i];
		parent[i] = 0;
	}
  */
  thrust::for_each(indexSequenceDev.begin(), indexSequenceDev.end(), compute_min_dist_functor(&villesPosVecDev, 0));
  thrust::for_each(villesPosVecDev.begin(), villesPosVecDev.end(), set_all_parent_functor(0));
  villesPosVecDev[0].is_in = true;


	//iteration of Prime
	
  int k;
  distance_total = 0;
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
    POS *minPosPtr = thrust::min_element(indexSequenceDev.begin(), indexSequenceDev.end(), compare_min_dist_functor());
    
    minPosPtr->is_in = true;
    distance_total += minPosPtr->min_dist;
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
    thrust::for_each(indexSequenceDev.begin(), indexSequenceDev.end(), update_min_dist_functor(&villesPosVecDev, minPosPtr->index));
  }
   thrust::copy(villesPosVecDev.begin(), villesPosVecDev.end(), villesPosVecHost.begin());

}


#endif
