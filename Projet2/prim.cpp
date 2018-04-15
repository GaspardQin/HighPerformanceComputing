#ifndef PRIME_CPP
#define PRIME_CPP
#include "prim.h"
using namespace std;
const float pi = std::acos(-1);

float sin_deg(float a){
        return (sinf(a/180*pi));
}
float cos_deg(float a){
        return (cosf(a/180*pi));
}

void computeDistance(float* &villesLon, float* &villesLat, const int nbVilles, float** &distance)
{
	int i, j;

	// inital
	distance = new float* [nbVilles];
  //distance = (float**) malloc(nbVilles * sizeof(float*));
  float* sin_lat = new float[nbVilles];
  //float* sin_lat = (float *) aligned_alloc(32,nbVilles * sizeof(float));
  float* cos_lat = new float[nbVilles];
  //float* cos_lat = (float *) aligned_alloc(32,nbVilles * sizeof(float));
  //#pragma omp parallel for simd num_threads(4) schedule(dynamic,4)

	for(i = 0; i < nbVilles; i++)
	{
		distance[i] = new float[nbVilles];
    //distance[i] = (float*) malloc(nbVilles * sizeof(float));
	}
  #pragma ivdep
  for(i = 0; i < nbVilles; i++){
    sin_lat[i] = sin_deg(villesLat[i]);
    cos_lat[i] = cos_deg(villesLat[i]);
  }
	// compute distance
  int jj,ii; int jj_max, ii_max;

  float distance_temp_block;
  #pragma omp parallel for num_threads(4) schedule(dynamic,4)
	for(i = 0; i < nbVilles; i+= BLOCK_SIZE)
	{
		for( j = 0; j <= i; j += BLOCK_SIZE){
      ii_max = min(nbVilles, i + BLOCK_SIZE);

      for(ii = i; ii < ii_max; ii++){
        jj_max = min(ii+1, j + BLOCK_SIZE);
        #pragma simd
        for(jj = j; jj < jj_max; jj++ ){
          //cout<<"ii: "<<ii <<" jj:"<<jj<<endl;
              //distance_temp_block[jj-j] =  R * acos( sin_lat[i] * sin_lat[jj]  + cos_deg(villesLon[i]-villesLon[jj]) * cos_lat[i]* cos_lat[jj]);
          distance_temp_block =  R * acosf( sin_lat[ii] * sin_lat[jj]  + cos_deg(villesLon[ii]-villesLon[jj]) * cos_lat[ii]* cos_lat[jj]);
          //cout<<"ii: "<<ii <<" jj:"<<jj : <<endl;

          distance[ii][jj] = distance_temp_block;
          distance[jj][ii] = distance_temp_block;
        }
      }
    }
	}

}

void prim(float* &villesLon, float* &villesLat, const int nbVilles, int *&parent,float** &distance)
{
	// parent[i] = j means j is the parent node of i
	computeDistance(villesLon, villesLat, nbVilles, distance);

	// define variables
	bool* inS = new bool [nbVilles];
  //bool* inS = (bool *)malloc(nbVilles * sizeof(bool));
	float* min_dist = new float [nbVilles];
  //float* min_dist = (float*)malloc(nbVilles * sizeof(float));
  parent = new int[nbVilles];
	//parent =  (int *)malloc(nbVilles * sizeof(int));
	int i, j;
	// init Prime
	inS[0] = true;
	min_dist[0] = 0;
	parent[0] = 0;
	for(i = 1; i < nbVilles; i++)
	{
		inS[i] = false;
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
	float min_min_dist = FLT_MAX;
	for(k = 0; k < nbVilles -1 ; k++)
	{
		// find the minimal min_dist outstide of S
		min_min_dist = FLT_MAX;

		for(i = 0; i < nbVilles; i++)
		{
				if(min_dist[i] >0 && min_min_dist > min_dist[i])
				{
					min_min_dist = min_dist[i];
					min_min_dist_index = i;
				}
		}
		inS[min_min_dist_index] = true;
    min_dist[min_min_dist_index] = -1;
		//update the min_dist
    int dist_temp;

		for(j = 0; j < nbVilles; j++)
		{
      dist_temp = distance[min_min_dist_index][j];

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

}

void showAllDistance(float* &villesLon, float* &villesLat, int &nbVilles, int *&parent, float** &distance){
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
