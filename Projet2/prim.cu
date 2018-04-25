#ifndef PRIME_CPP
#define PRIME_CPP
#include "prim.h"
using namespace std;
double pi = std::acos(-1);

#define deg_to_rad_fac (M_PI / 180.0)

__host__ __device__
inline thrust::complex<double> sin_deg(thrust::complex<double> a){
        return (thrust::sin(a * deg_to_rad_fac));
}
__host__ __device__
inline thrust::complex<double> cos_deg(thrust::complex<double> a){
        return (thrust::cos(a * deg_to_rad_fac));
}


__host__ __device__
inline double getDistance(double lat1, double lon1, double lat2, double lon2){
  thrust::complex<double> complex_lat1 = thrust::complex<double>(lat1);
  thrust::complex<double> complex_lat2 = thrust::complex<double>(lat2);
  thrust::complex<double> complex_lon1 = thrust::complex<double>(lon1);
  thrust::complex<double> complex_lon2 = thrust::complex<double>(lon2);

  return  R_EARTH * thrust::norm(thrust::acos( thrust::sin(complex_lat1) * thrust::sin(complex_lat2) 
             + cos_deg(complex_lon1 - complex_lon2) * thrust::cos(complex_lat1)* thrust::cos(complex_lat2)));
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
  thrust::device_vector<POS>::iterator pos_ptr;
  int new_in_index;
  double temp;
  POS pos_a;
  POS pos_new_index;
  update_min_dist_functor(thrust::device_vector<POS>::iterator pos_ptr_, int new_in_index_): pos_ptr(pos_ptr_), new_in_index(new_in_index_){
    pos_new_index = *(pos_ptr+new_in_index);
  };
  
  __host__ __device__
  void operator()(const int& a)
  {
    pos_a = *(pos_ptr+a);
    if(pos_a.is_in == true)
      return;
    else{
      temp = getDistance(pos_a.lat, pos_a.lon, pos_new_index.lat,pos_new_index.lon);
      if(temp < pos_a.min_dist){
        pos_a.parent = new_in_index;
        *(pos_ptr+a) = pos_a;
      }
    }
  }
};


struct set_all_parent_functor
{
    int parent_default;
    
    set_all_parent_functor(int a):parent_default(a){};
    
    __host__ __device__
    void operator()(POS& A){ 
      A.parent = parent_default;    
    }
};
struct compute_min_dist_functor{
    thrust::device_vector<POS>::iterator pos_ptr;
    int compare_index;
    POS pos_compare_copy;
    POS pos_a_copy;
    compute_min_dist_functor(thrust::device_vector<POS>::iterator pos_ptr_ , int compare_index_){
      pos_ptr = pos_ptr_;
      compare_index = compare_index_;
      pos_compare_copy = *(pos_ptr+compare_index);
    }
    
    __host__ __device__
        void operator()(const int& a){
            pos_a_copy = *(pos_ptr + a); 
            pos_a_copy.min_dist = getDistance(pos_a_copy.lat, pos_a_copy.lon, pos_compare_copy.lat,pos_compare_copy.lon);
            *(pos_ptr + a) = pos_a_copy;
        }
};


struct compare_min_dist_functor
{
  
  __device__  bool operator()(const POS &a, const POS &b)
  {
    return (b.min_dist +b.is_in * FLT_MAX > a.min_dist + a.is_in * FLT_MAX);
    
    //return thrust::greater<double>()(b.min_dist +b.is_in * FLT_MAX,a.min_dist + a.is_in * FLT_MAX);
  }
};

void prim(thrust::host_vector<POS> &villesPosVecHost,const int nbVilles, double &distance_total)
{
  // copy to GPU memory
  cout << "start to copy GPU memory"<<endl;
  thrust::device_vector<POS> villesPosVecDev(nbVilles);
  cout <<"checkpoint1"<<endl;

  thrust::copy(villesPosVecHost.begin(), villesPosVecHost.end(), villesPosVecDev.begin());
  cout <<"checkpoint2"<<endl;
  thrust::host_vector<int> indexSequenceHost(nbVilles);
  thrust::sequence(indexSequenceHost.begin(), indexSequenceHost.end());
    cout <<"checkpoint4"<<endl;

  thrust::device_vector<int> indexSequenceDev(nbVilles);
  thrust::copy(indexSequenceHost.begin(), indexSequenceHost.end(), indexSequenceDev.begin());

  cout << "GPU memory copied" <<endl;
	// init Prime 
  /* 
	for(i = 1; i < nbVilles; i++)
	{
		min_dist[i] = distance[0][i];
		parent[i] = 0;
	}
  */
  thrust::for_each(indexSequenceDev.begin(), indexSequenceDev.end(), compute_min_dist_functor(villesPosVecDev.begin(), 0));
  thrust::for_each(villesPosVecDev.begin(), villesPosVecDev.end(), set_all_parent_functor(0));
  POS temp_pos = villesPosVecDev[0];
  temp_pos.is_in = true;
  villesPosVecDev[0] = temp_pos;


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
    thrust::device_vector<POS>::iterator minPosPtr = thrust::min_element(thrust::device,villesPosVecDev.begin(), villesPosVecDev.end(), compare_min_dist_functor());
    temp_pos = *minPosPtr;
    temp_pos.is_in = true;
    *minPosPtr = temp_pos;
    distance_total += temp_pos.min_dist;
    cout << "min_dist: "<< temp_pos.min_dist << " distance_total: "<< distance_total<<endl;
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
    thrust::for_each(indexSequenceDev.begin(), indexSequenceDev.end(), update_min_dist_functor(villesPosVecDev.begin(), temp_pos.index));
  }
   thrust::copy(villesPosVecDev.begin(), villesPosVecDev.end(), villesPosVecHost.begin());

}


#endif
