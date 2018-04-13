#ifndef PRIME_CPP
#define PRIME_CPP
#include "prim.h"
using namespace std;
const double pi = std::acos(-1);

float sin_deg(float a){
        return (sin(a/180*pi));
}
float cos_deg(float a){
        return (cos(a/180*pi));
}

void computeDistance(float* &villesLon, float* &villesLat, const int nbVilles, float** &distance)
{
	int i, j;
	float dist_temp;

	// inital
	distance = new float* [nbVilles];
  float* sin_lat = new float[nbVilles];
  float* cos_lat = new float[nbVilles];

  //#pragma omp parallel for simd num_threads(4) schedule(dynamic,4)

	for(i = 0; i < nbVilles; i++)
	{
		distance[i] = new float[i+1];

	}
  #pragma ivdep
  for(i = 0; i < nbVilles; i++){
    sin_lat[i] = sin_deg(villesLat[i]);
    cos_lat[i] = cos_deg(villesLat[i]);
  }
	// compute distance
  #pragma omp parallel for num_threads(4) schedule(dynamic,1)
	for(i = 0; i < nbVilles; i++)
	{
		distance[i][i] = 0;
    #pragma ivdep
		for( j = 0; j <  i; j++){
			distance[i][j] =  R * acos( sin_lat[i] * sin_lat[j]  + cos_deg(villesLon[i]-villesLon[j]) * cos_lat[i]* cos_lat[j]);

		}
	}

}
float getDistance(int i, int j, float** &distance){
        if(j <= i)
                return distance[i][j];
        else
                return distance[j][i];

}
void prim(float* &villesLon, float* &villesLat, const int nbVilles, int *&parent,float** &distance)
{
	// parent[i] = j means j is the parent node of i
	computeDistance(villesLon, villesLat, nbVilles, distance);

	// define variables
	bool* inS = new bool [nbVilles];
	float* min_dist = new float [nbVilles];
	parent = new int [nbVilles];
	int i, j;
	// init Prime
	inS[0] = true;
	min_dist[0] = 0;
	parent[0] = 0;
	for(i = 1; i < nbVilles; i++)
	{
		inS[i] = false;
		min_dist[i] = distance[i][0];
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
        fileOut << parent[i] << " "<< i <<" "<< getDistance(i,parent[i],distance)<< " " << int(inS[i]) <<"\n";
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
				if(inS[i] == false && min_min_dist > min_dist[i])
				{
					min_min_dist = min_dist[i];
					min_min_dist_index = i;
				}
		}
		inS[min_min_dist_index] = true;
		//update the min_dist
		for(j = 0; j < nbVilles; j++)
		{
			if(inS[j] == false && min_dist[j] > getDistance(min_min_dist_index,j,distance))
			{
				min_dist[j] = getDistance(min_min_dist_index,j,distance);
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
                        fileOut << parent[i] << " "<< i <<" "<< getDistance(i,parent[i],distance)<< " " << int(inS[i]) <<"\n";
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
			fileOut << i << " "<< j <<" "<< getDistance(i,j,distance)<< "\n";
		}
	}
	fileOut.close();
}


#endif
