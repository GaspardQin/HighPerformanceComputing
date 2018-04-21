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
std::ofstream log_stream;
int main(int argc, char** argv) {

//-----------------------------------------------------------------
//--- LECTURE des villes
//-------------------t
#ifdef DEBUG_LOG
    if(argc != 2){
      cout << "wrong input!"<<endl;
      cout << "You should input log file name as an argument"<<endl;
      return 1;
    } 
    else{  
      log_stream.open(argv[1]);
      if(log_stream.is_open()){
        cout << "successfully created log file : "<<argv[1]<<endl;
      }
      else{
        cout << "failed to create log file : "<<argv[1]<<endl;
        return 1;
      }
    }
#endif
  int popMin;
  #ifdef ADVIXE
  popMin = 0;//10000;//3000;

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
  //cout << "301 " << villesNom[301]<< " lon" << villesLon[301] << " lat " << villesLat[301] << endl;

  //cout << "305 " <<villesNom[305]<< " lon" << villesLon[305] << " lat " << villesLat[305] << endl;
  // ... juste pour vérifier !  (Vous pouvez retirer cette ligne.)
  //for(int i=0; i<N; i++)
  //  cout << villesNom[i] << " " << villesPop[i] << " " << villesLon[i] << " " << villesLat[i] << endl;

//-----------------------------------------------------------------
//--- CALCUL du graphe
//-----------------------------------------------------------------

  // Début du CHRONO
  high_resolution_clock::time_point timeStart = high_resolution_clock::now();


  // [...]
  int * graphe;
  float distance_total;

  prim(villesLon, villesLat, N, graphe, distance_total);


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
    fileOut << graphe[i] << " "<< i << "\n";
  }

  fileOut.close();
  _mm_free(graphe);

  delete[] villesNom;
  //_mm_free(villesNom);
  _mm_free(villesPop);
  _mm_free(villesLon);
  _mm_free(villesLat);
//-----------------------------------------------------------------
//--- DESALLOCATION des tableaux
//-----------------------------------------------------------------
  #ifdef DEBUG_LOG
  log_stream.close();
  #endif
  return 0;
}
