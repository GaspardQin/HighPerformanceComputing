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

void computeDistance(float* &villesLon, float* &villesLat, int &nbVilles, float** &distance)
{
	int i, j;
	float dist_temp;

	// inital
	distance = new float* [nbVilles];
	for(i = 0; i < nbVilles; i++)
	{
		distance[i] = new float[nbVilles];
	}
	// compute distance
	for(i = 0; i < nbVilles; i++)
	{
		distance[i][i] = 0;
		for( j = i+1; j <  nbVilles; j++){
			dist_temp =  R * acos( sin_deg(villesLat[i]) * sin_deg(villesLat[j]) + cos_deg(villesLon[i]-villesLon[j]) * cos_deg(villesLat[i])*cos_deg(villesLat[j]));
			distance[i][j] = dist_temp;
			distance[j][i] = dist_temp;
		}
	}

}

void prim(float* &villesLon, float* &villesLat, int &nbVilles, int *&parent,float** &distance)
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
			if(inS[j] == false && min_dist[j] > distance[min_min_dist_index][j])
			{
				min_dist[j] = distance[min_min_dist_index][j];
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
