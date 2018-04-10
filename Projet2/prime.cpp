#ifndef PRIME_CPP
#define PRIME_CPP
#define R 6378
using namespace std;
void computeDistance(float* &villesLon, float* &villesLat, int &nbVilles, float** distance)
{
	int i, j;
	float dist_temp;
	
	// inital
	distance = new float* [nbVilles];
	for(i = 0; i < nbVilles; i++)*
	{
		distance[i] = new float[nbVilles];
	}	
	
	// compute distance
	for(i = 0; i < nbVilles; i++)
	{
		distance[i][j] = 0;
		for( j = i+1; j <  nbVilles; j++){
			dist_temp =  R * acos( sin(villesLat[i]) * sin(villesLat[j]) + cos(villesLon[i]-villesLon[j]) * cos(villesLat[i])*cos(villesLat[j]));
			distance[i][j] = dist_temp;
			distance[j][i] = dist_temp;
		}			
	}

}

void Prim(float* &villesLon, float* &villesLat, int &nbVilles, int *graphe)
{
	// graphe[i] = j means j is the parent node of i
	float ** distance;
	computeDistance(villesLon, villesLat, nbVilles, distance);
	
	// define variables
	inS = new bool [nbVilles];
	min_dist = new float [nbVilles];
	graphe = new int [nbVilles];
	
	int i, j;
	// init Prime
	inS[0] = true;
	min_dist[0] = 0;
	for(i = 1; i < nbVilles; i++)
	{
		inS[i] = false;
		min_dist[i] = distance[0][i];
		graphe[i] = 0;
	}
	
	//iteration of Prime
	int k, min_min_dist_index;
	float min_min_dist;
	for(k = 0; k < nbVilles -1 ; k++)
	{
		// find the minimal min_dist outstide of S
		for(i = 0; i < nbVilles)
		
	}
}

#endif