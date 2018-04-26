#ifndef PRIME_H
#define PRIME_H
#define R_EARTH 6378
#include <cmath>
#include <iostream>
#include <float.h>
#include <fstream>
#include <sstream>

#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <stdlib.h>
//#include <thrust/host_vector.h>
//#include <thrust/device_vector.h>
//#include <thrust/extrema.h>
//#include <thrust/complex.h>
#include <cuda_runtime.h>
#if defined(__MIC__)
#define VEC_ALIGN 64
#else
#define VEC_ALIGN 32
#endif


//#define SHOW_ALL
//#define SHOW_EVERY_STEPS
#define BLOCK_SIZE 32

#define ADVIXE

/*
class POS{
  public:
    double lat;
    double lon;
    double min_dist;
    int parent;
    int index;
    bool is_in;
    POS(){
      lat=0;
      lon=0;
      min_dist = -1;
      parent = -1;
      index = 0;
      is_in = false;
    }
    POS(double lat_, double lon_, int index_): lat(lat_), lon(lon_), index(index_)
    {
      min_dist = -1;
      parent = -1;
    };
};  
*/

void prim(float *lat_host, float* lon_host, int* parent_host,
          const int nbVilles, float *distance_total);


template <typename T0,typename T1,typename T2>
void debug_print(T0 * array0 , T1 * array1, T2 num, int size){
  for(int i =0; i<size; i++)
  {
    std::cout<< array0[i] << " "<< array1[i] << " "<< num<< std::endl;
  }
}
#endif
