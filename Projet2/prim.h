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
#include <unordered_map>

//#define SHOW_ALL
//#define SHOW_EVERY_STEPS
#define ADVIXE
void computeDistance(float* &villesLon, float* &villesLat, int nbVilles, float** &distance);
void prim(float* &villesLon, float* &villesLat,const int nbVilles, int *&parent, float** &distance);
void showAllDistance(float* &villesLon, float* &villesLat, int &nbVilles, int *&parent, float** &distance);
#endif
