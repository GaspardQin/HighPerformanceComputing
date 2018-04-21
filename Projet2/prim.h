#ifndef PRIME_H
#define PRIME_H
#define R 6378
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
//#if defined(__MIC__)
#define VEC_ALIGN 32
//#else
//#define VEC_ALIGN 32
//#endif


//#define SHOW_ALL
//#define SHOW_EVERY_STEPS
#define BLOCK_SIZE 32

//  #define ADVIXE
void computeDistance(double* &villesLon, double* &villesLat, int nbVilles, double** &distance);
void prim(double* &villesLon, double* &villesLat,const int nbVilles, int *&parent, double &distance_total);
void showAllDistance(double* &villesLon, double* &villesLat, int &nbVilles, int *&parent, double** &distance);

class MinMinDistType{
public:
  int index;
  double distance;

  bool operator< (MinMinDistType a2){
    if(a2.distance > this->distance)
      return true;
    else
      return false;
  }
};


#endif
