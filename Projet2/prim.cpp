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
	//distance = new float* [nbVilles];
  distance = (float**)_mm_malloc(nbVilles * sizeof(float*), VEC_ALIGN);
  //float* sin_lat = new float[nbVilles];
  float* sin_lat = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
  //float* cos_lat = new float[nbVilles];
  float* cos_lat = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
  //#pragma omp parallel for simd num_threads(4) schedule(dynamic,4)

	for(i = 0; i < nbVilles; i++)
	{
    __assume_aligned(distance, VEC_ALIGN);
		distance[i] = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
    //distance[i] = (float*) malloc(nbVilles * sizeof(float));
	}
  #pragma ivdep
  for(i = 0; i < nbVilles; i++){
    __assume_aligned(villesLat, VEC_ALIGN);
    sin_lat[i] = sin_deg(villesLat[i]);
    cos_lat[i] = cos_deg(villesLat[i]);
  }
	// compute distance
  int jj,ii; int jj_max, ii_max;
  float* distance_ii_ptr;
  float distance_temp_block;
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
          distance_temp_block =  R * acosf( sin_lat[ii] * sin_lat[jj]  + cos_deg(villesLon[ii]-villesLon[jj]) * cos_lat[ii]* cos_lat[jj]);
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
void computeCosSin(float* &sin_lat, float* &cos_lat,const float* villesLat, const int nbVilles, int start_index){
    sin_lat = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
    cos_lat = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
    #pragma ivdep
    for(int i = 0; i < nbVilles; i++){
      __assume_aligned(villesLat, VEC_ALIGN);
      sin_lat[i] = sin_deg(villesLat[start_index+i]);
      cos_lat[i] = cos_deg(villesLat[start_index+i]);
    }
}

void prim(float* &villesLon, float* &villesLat, const int nbVilles, int *&parent,double & distance_total, int start_index)
{
	// parent[i] = j means j is the parent node of i
  float * sin_lat, * cos_lat;
	computeCosSin(sin_lat, cos_lat, villesLat, nbVilles, start_index);
  distance_total = 0;
	// define variables
	//bool* inS = new bool [nbVilles];
  //bool* inS = (bool *)malloc(nbVilles * sizeof(bool));
	//float* min_dist = new float [nbVilles];
  float* min_dist = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
  //float* min_dist = (float*)malloc(nbVilles * sizeof(float));
  //parent = new int[nbVilles];


	//parent =  (int *)malloc(nbVilles * sizeof(int));
	int i, j;
	// init Prime
	//inS[0] = true;
	min_dist[0] = -1;
	parent[start_index] = start_index;

	for(i = 1; i < nbVilles; i++)
	{
		//inS[i] = false;
		min_dist[i] = R * acosf( sin_lat[i] * sin_lat[0]  + cos_deg(villesLon[i+start_index]-villesLon[start_index]) * cos_lat[i]* cos_lat[0]);
		parent[i+start_index] = start_index;
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
	for(k = 1; k < nbVilles; k++)
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

    float min_min_dist_index_sin_lat = sin_lat[min_min_dist_index];
    float min_min_dist_index_cos_lat = cos_lat[min_min_dist_index];
    float min_min_dist_index_villes_lon = villesLon[min_min_dist_index+start_index];

    //#pragma omp parallel for simd num_threads(4) schedule(dynamic,8) aligned(sin_lat:VEC_ALIGN, villesLon:VEC_ALIGN, cos_lat:VEC_ALIGN, min_dist:VEC_ALIGN,parent:VEC_ALIGN) firstprivate(min_min_dist_index_sin_lat, min_min_dist_index_cos_lat, min_min_dist_index_villes_lon) private(dist_temp)
		for(j = 0; j < nbVilles; j++)
		{
      __assume_aligned(sin_lat, VEC_ALIGN);
      __assume_aligned(villesLon, VEC_ALIGN);
      __assume_aligned(cos_lat, VEC_ALIGN);
      float dist_temp;
      dist_temp =  R * acosf( min_min_dist_index_sin_lat * sin_lat[j]  + cos_deg(min_min_dist_index_villes_lon-villesLon[j+start_index]) * min_min_dist_index_cos_lat* cos_lat[j]);

			if(min_dist[j] > dist_temp)
			{

				min_dist[j] = dist_temp;
				parent[start_index+j] = min_min_dist_index+start_index;
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
  _mm_free(cos_lat);
  _mm_free(sin_lat);
}

void primeDepartement(float* &villesLon, float* &villesLat, int &nbVilles, int * &villesPop, int *&parent,
  int* &beginDeparte,  int* &rootDepartement , float *&maxVillesLon,float *&maxVillesLat, int *&maxVillesParent, double & distance_total){
  rootDepartement = (int*)_mm_malloc(NB_DEPART * sizeof(int),VEC_ALIGN); //begin at index 0
  maxVillesLon = (float*)_mm_malloc(NB_DEPART * sizeof(int),VEC_ALIGN);
  maxVillesLat = (float*)_mm_malloc(NB_DEPART * sizeof(int),VEC_ALIGN);

  int depart;
  double distance_depart_total= 0;
  for(depart = 0; depart < NB_DEPART-1; depart++){
    int index = beginDeparte[depart];
    int nbVillesDepart = beginDeparte[depart+1] - beginDeparte[depart];
    double distance_depart;

    //find the max pop city
    int end_depart_index = beginDeparte[depart+1] -1;
    cout<<"depart : "<<depart <<" end_index: "<<end_depart_index<<endl;
    int max_pop =0;
    int max_index = 0;
    int i;
    for( i= index; i < end_depart_index; i++){
      //cout << "i, end_depart_index: "<< i << " , "<< end_depart_index<<endl;
      //cout << "villesPop :" << villesPop[i] << " max_pop: "<<max_pop<<endl;
      if(villesPop[i] > max_pop){
        max_pop = villesPop[i];
        max_index = i;
      }
    }

    rootDepartement[depart] = max_index;
    maxVillesLon[depart] = villesLon[max_index];
    maxVillesLat[depart] = villesLat[max_index];
    cout << "max_index: "<<max_index<<endl;
    cout<< "call prim: index:"<<index<<" nbVillesDepart: " <<nbVillesDepart <<endl;
    prim(villesLon, villesLat, nbVillesDepart, parent,distance_depart, index);
    distance_depart_total += distance_depart;


    #ifdef SHOW_EVERY_DEPARTEMENT
                // create files steps files to store data
                string save_name_base = "./steps/resuGraphe_";

                cout<<"depart: "<< depart<<endl;
                string save_name = save_name_base + to_string(depart) + ".dat";
                ofstream fileOut(save_name);
                for(int i =index; i <= end_depart_index; i++)
                {
                        fileOut << parent[i] << " "<< i  <<"\n";
                }
                fileOut.close();
    #endif


  }

  // prim of the max city of each departement;
  double distance_between_depart_total = 0;
  prim(maxVillesLon, maxVillesLat, NB_DEPART-1, maxVillesParent ,distance_between_depart_total,0);
  distance_total = distance_depart_total + distance_between_depart_total;
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
