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
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/extrema.h>
#include <thrust/complex.h>
#if defined(__MIC__)
#define VEC_ALIGN 64
#else
#define VEC_ALIGN 32
#endif


//#define SHOW_ALL
//#define SHOW_EVERY_STEPS
#define BLOCK_SIZE 32

#define ADVIXE


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

void computeDistance(float* &villesLon, float* &villesLat, int nbVilles, float** &distance);
void prim(thrust::host_vector<POS> &villesPosVecHost,const int nbVilles, double &distance_total);
void showAllDistance(float* &villesLon, float* &villesLat, int &nbVilles, int *&parent, float** &distance);



#endif
