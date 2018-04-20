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
#include <malloc.h>
//#if defined(__MIC__)
#define VEC_ALIGN 32
//#else
//#define VEC_ALIGN 32
//#endif


//#define SHOW_ALL
//#define SHOW_EVERY_STEPS
//#define SHOW_EVERY_DEPARTEMENT
#define BLOCK_SIZE 32
#define NB_DEPART 95
#define ADVIXE
void prim(float* &villesLon, float* &villesLat,const int nbVilles, int *&parent, double &distance_total, int start_index);
void primeDepartement(float* &villesLon, float* &villesLat, int &nbVilles, int * &villesPop, int *&parent,
  int* &beginDeparte,  int* &rootDepartement , float *&maxVillesLon,float *&maxVillesLat, int *&maxVillesParent, double & distance_total);

#endif
