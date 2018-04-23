#ifndef PRIME_CPP
#define PRIME_CPP
#include "prim.h"
using namespace std;
const double pi = std::acos(-1);
const double deg_to_rad_fac = pi/180.0;
inline double sin_deg(double a){
        return (sin(a * deg_to_rad_fac));
}
inline double cos_deg(double a){
        return (cos(a * deg_to_rad_fac));
}

void computeDistance(double* &villesLon, double* &villesLat, const int nbVilles, double** &distance)
{
	int i, j;

	// inital
	//distance = new double* [nbVilles];
  distance = (double**)_mm_malloc(nbVilles * sizeof(double*), VEC_ALIGN);
  //double* sin_lat = new double[nbVilles];
  double* sin_lat = (double*)_mm_malloc(nbVilles * sizeof(double),VEC_ALIGN);
  //double* cos_lat = new double[nbVilles];
  double* cos_lat = (double*)_mm_malloc(nbVilles * sizeof(double),VEC_ALIGN);
  //#pragma omp parallel for simd num_threads(4) schedule(dynamic,4)

	for(i = 0; i < nbVilles; i++)
	{
    __assume_aligned(distance, VEC_ALIGN);
		distance[i] = (double*)_mm_malloc(nbVilles * sizeof(double),VEC_ALIGN);
    //distance[i] = (double*) malloc(nbVilles * sizeof(double));
	}
  #pragma ivdep
  for(i = 0; i < nbVilles; i++){
    __assume_aligned(villesLat, VEC_ALIGN);
    sin_lat[i] = sin_deg(villesLat[i]);
    cos_lat[i] = cos_deg(villesLat[i]);
  }
	// compute distance
  int jj,ii; int jj_max, ii_max;
  double* distance_ii_ptr;
  double distance_temp_block;
  const double f_1 = 1.0;
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
          distance_temp_block =  R * acos( min(sin_lat[ii] * sin_lat[jj]  + cos_deg(villesLon[ii]-villesLon[jj]) * cos_lat[ii]* cos_lat[jj], f_1 ));
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

void prim(double* &villesLon, double* &villesLat, const int nbVilles, int *&parent,double** &distance, double &distance_total)
{
	// parent[i] = j means j is the parent node of i
	computeDistance(villesLon, villesLat, nbVilles, distance);

	// define variables
	//bool* inS = new bool [nbVilles];
  //bool* inS = (bool *)malloc(nbVilles * sizeof(bool));
	//double* min_dist = new double [nbVilles];
  double* min_dist = (double*)_mm_malloc(nbVilles * sizeof(double),VEC_ALIGN);
  //double* min_dist = (double*)malloc(nbVilles * sizeof(double));
  //parent = new int[nbVilles];
  parent = (int*)_mm_malloc(nbVilles * sizeof(int),VEC_ALIGN);
  distance_total = 0.0;
	//parent =  (int *)malloc(nbVilles * sizeof(int));
	int i, j;
	// init Prime
	//inS[0] = true;
	min_dist[0] = -1;
	parent[0] = 0;
	for(i = 1; i < nbVilles; i++)
	{
		//inS[i] = false;
		min_dist[i] = distance[0][i];
		parent[i] = 0;
	}

        #ifdef SHOW_EVERY_STEPS
        // check and create folder "steps"
        int iRet = access("./steps",0);
        if (iRet != 0)
                iRet = mkdir("./steps",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        // create files steps files to store data
        string save_name_base = "./steps/resuGraphe_";
        string save_name = save_name_base + "0.dat";
        //cout<<"steps: "<< 0<<endl;
        ofstream fileOut(save_name);
        for(int i =0; i < nbVilles; i++)
        {
        fileOut << parent[i] << " "<< i <<" "<< distance[i][parent[i]]<< " " << int(inS[i]) <<"\n";
        }
        fileOut.close();
        #endif // SHOW_EVERY_STEPS

	//iteration of Prime
	int k;
	int min_min_dist_index;
	double min_min_dist = FLT_MAX;
	for(k = 0; k < nbVilles -1 ; k++)
	{
		// find the minimal min_dist outstide of S
		min_min_dist = FLT_MAX;

		for(i = 0; i < nbVilles; i++)
		{
      __assume_aligned(min_dist, VEC_ALIGN);
				if(min_dist[i] >=0 && min_min_dist > min_dist[i])
				{
					min_min_dist = min_dist[i];
					min_min_dist_index = i;
				}
		}
		//inS[min_min_dist_index] = true;
    min_dist[min_min_dist_index] = -1;
    distance_total += min_min_dist;
		//update the min_dist
    double dist_temp;

    double* distance_i_ptr =  distance[min_min_dist_index];
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
		#ifdef SHOW_EVERY_STEPS
                // create files steps files to store data

                cout<<"steps: "<< k+1<<endl;
                save_name = save_name_base + to_string(k+1) + ".dat";
                ofstream fileOut(save_name);
                for(int i =0; i < nbVilles; i++)
                {
                        fileOut << parent[i] << " "<< i <<" "<< distance[i][parent[i]]<< " " << int(inS[i]) <<"\n";
                }
                fileOut.close();
		#endif

	}
  _mm_free(min_dist);
}

void showAllDistance(double* &villesLon, double* &villesLat, int &nbVilles, int *&parent, double** &distance){
	// parent[i] = j means j is the parent node of i

	computeDistance(villesLon, villesLat, nbVilles, distance);

	ofstream fileOut("resuGrapheAll.dat");
	for(int i =0; i < nbVilles; i++)
	{
		for(int j = i + 1; j < nbVilles; j++){
			fileOut << i << " "<< j <<" "<< distance[i][j]<< "\n";
		}
	}
	fileOut.close();
}


#endif
