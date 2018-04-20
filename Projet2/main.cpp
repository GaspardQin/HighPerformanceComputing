// g++ citiesPlot.py

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>

using namespace std;
using namespace std::chrono;

#include "lectureVilles.h"
#include "prim.h"

int main() {

//-----------------------------------------------------------------
//--- LECTURE des villes
//-----------------------------------------------------------------

  int popMin;
  #ifdef ADVIXE
  popMin = 0;

  #else
  cout << "Population minimale ? ";
  cin >> popMin;
  #endif
  string* villesNom; // Nom
  int*    villesPop; // Population
  float*  villesLon; // Longitude
  float*  villesLat; // Latitude
  int     N;         // Nombre de villes
  int*    villesDeparte;
  int*    beginDeparte;
  lectureVilles(popMin, villesNom, villesPop, villesLon, villesLat,villesDeparte,beginDeparte, N);
  //cout << "301 " << villesNom[301]<< " lon" << villesLon[301] << " lat " << villesLat[301] << endl;

  //cout << "305 " <<villesNom[305]<< " lon" << villesLon[305] << " lat " << villesLat[305] << endl;
  // ... juste pour vérifier !  (Vous pouvez retirer cette ligne.)
  for(int i=0; i<N; i++)
    cout << villesNom[i] << " " <<villesDeparte[i]<<" "<< villesPop[i] << " " << villesLon[i] << " " << villesLat[i] << endl;

//-----------------------------------------------------------------
//--- CALCUL du graphe
//-----------------------------------------------------------------

  // Début du CHRONO
  high_resolution_clock::time_point timeStart = high_resolution_clock::now();


  // [...]
  double distance_total;
  int* graphe_in_depart = (int*)_mm_malloc(N * sizeof(int),VEC_ALIGN);
  int* graphe_between_depart = (int*)_mm_malloc(NB_DEPART * sizeof(int),VEC_ALIGN);//begin at index 0
  //prim(villesLon, villesLat, N, graphe, distance_total);

  int *rootDepartement;
  float* maxVillesLat;
  float* maxVillesLon;
  int* maxVillesGraphe;
  primeDepartement(villesLon,villesLat, N, villesPop, graphe_in_depart,
    beginDeparte,  rootDepartement , maxVillesLon, maxVillesLat, graphe_between_depart, distance_total);



  // Fin du CHRONO
  high_resolution_clock::time_point timeEnd = high_resolution_clock::now();
  duration<double> timeSpan = duration_cast<duration<double>>(timeEnd-timeStart);
  double timeTotal = timeSpan.count();
  cout<< endl;
  cout<< "Total time: " << timeTotal << endl;
    cout.precision(5);
  cout << "distance total: "<<fixed<< distance_total << endl;
  #ifdef SHOW_ALL
  showAllDistance(villesLon, villesLat, N, graphe, distance);
  #endif
  // Écriture du graphe (chaque ligne correspond à une arête)
  // !!! Ci-dessous, on écrit le graphe complet pour l'exemple
  // !!! Vous devez modifier cette commande pour écrire le graphe obtenu avec Prim
  ofstream fileOut("resuGraphe.dat");
  //for(int i=0; i<N; i++)
  //  for(int j=0; j<i; j++)
  //    fileOut << i << " " << j << "\n";
  for(int i =0; i < N; i++)
  {
    fileOut << graphe_in_depart[i] << " "<< i  <<" "<< villesDeparte[i]<< "\n";
  }
  for(int i=0; i < NB_DEPART-1; i++ ){
    fileOut << rootDepartement[graphe_between_depart[i]] << " "<<rootDepartement[i]<< " 0"<<"\n";
  }

  fileOut.close();
  _mm_free(graphe_in_depart);
  _mm_free(graphe_between_depart);
  delete[] villesNom;
  //_mm_free(villesNom);
  _mm_free(villesPop);
  _mm_free(villesLon);
  _mm_free(villesLat);
  _mm_free(villesDeparte);
  _mm_free(rootDepartement);
  _mm_free(beginDeparte);
  _mm_free(maxVillesLon);
  _mm_free(maxVillesLat);
//-----------------------------------------------------------------
//--- DESALLOCATION des tableaux
//-----------------------------------------------------------------

  return 0;
}
