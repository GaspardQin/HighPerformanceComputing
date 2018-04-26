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
  lectureVilles(popMin, villesNom, villesPop, villesLon, villesLat, N);

  // ... juste pour vérifier !  (Vous pouvez retirer cette ligne.)
 // for(int i=0; i<N; i++)
 //   cout << villesNom[i] << " " << villesPop[i] << " " << villesLon[i] << " " << villesLat[i] << endl;

//-----------------------------------------------------------------
//--- CALCUL du graphe
//-----------------------------------------------------------------



  // Début du CHRONO
  high_resolution_clock::time_point timeStart = high_resolution_clock::now();


  // [...]
  float distance_total;
  int *parent_host = new int[N];
  prim(villesLat, villesLon,parent_host, N,&distance_total);


  // Fin du CHRONO
  high_resolution_clock::time_point timeEnd = high_resolution_clock::now();
  duration<float> timeSpan = duration_cast<duration<float>>(timeEnd-timeStart);
  float timeTotal = timeSpan.count();
  cout<< endl;
  cout<< "Total time: " << timeTotal << endl;
  cout<< "distance total: " << distance_total << endl;
  // Écriture du graphe (chaque ligne correspond à une arête)
  // !!! Ci-dessous, on écrit le graphe complet pour l'exemple
  // !!! Vous devez modifier cette commande pour écrire le graphe obtenu avec Prim
  ofstream fileOut("resuGraphe.dat");
  //for(int i=0; i<N; i++)
  //  for(int j=0; j<i; j++)
  //    fileOut << i << " " << j << "\n";
  for(int i =0; i < N; i++)
  {
    fileOut << parent_host[i]<< " "<< i <<"\n";
  }
  fileOut.close();


  //_mm_free(villesNom);
  //_mm_free(villesPop);
  //_mm_free(villesLon);
  //_mm_free(villesLat);
  delete[] villesNom;
  delete[] villesPop;
  delete[] villesLon;
  delete[] villesLat;

//-----------------------------------------------------------------
//--- DESALLOCATION des tableaux
//-----------------------------------------------------------------

  return 0;
}
