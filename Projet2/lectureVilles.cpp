#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "lectureVilles.h"

using namespace std;

void lectureVilles(int popMin, string* &villesNom, int* &villesPop, float* &villesLon, float* &villesLat, int* &villesDeparte, int* &beginDeparte,int &nbVilles){

//-----------------------------------------------------------------
//--- LECTURE des villes de 'popMin' habitants ou plus
//-----------------------------------------------------------------

  cout << "== Lecture des villes de min. " << popMin << " habitants à partir de 'listeVilles.csv' ==" << endl;

  ifstream inputFile("listeVilles.csv");
  if (inputFile.is_open()) {

    // Compte le nombre de villes avec une population supérieure à 'popMin'
    nbVilles = 0;
    string line;
    while ( getline(inputFile,line) ) {
      int label = 0;
      stringstream ss;
      for(int i=0; i<line.size(); i++){
        if(line[i] == ',')
          label++;
        else{
          if(label==14) ss << line[i];
        }
      }
      string::size_type sz;
      int my_pop = stoi(ss.str(),&sz);
      if(my_pop >= popMin) nbVilles++;
    }

    // Allocation des tableaux
    villesNom = new string[nbVilles];
    //villesNom = (string*)_mm_malloc(nbVilles * sizeof(string),VEC_ALIGN);
    //villesPop = new int[nbVilles];
    villesPop = (int*)_mm_malloc(nbVilles * sizeof(int),VEC_ALIGN);
    //villesLon = new float[nbVilles];
    villesLon = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
    //villesLat = new float[nbVilles];
    villesLat = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
    villesDeparte = (int*)_mm_malloc(nbVilles * sizeof(int),VEC_ALIGN);
    beginDeparte = (int*) _mm_malloc((NB_DEPART+1) * sizeof(int),VEC_ALIGN); //start at index 0
    // Lecture/Sauvegarde des données

    int index=0;
    inputFile.clear();
    inputFile.seekg(0, ios::beg);
    int departe_curr = 0;
    int departe_count = 0; //to solve the problem that there does not exist departement 20
    while ( getline(inputFile,line) ) {

      int label=0;
      stringstream ss[5];
      for(int i=0; i<line.size(); i++){
        if(line[i] == ',')
          label++;
        else{
          if(label==1) ss[4] << line[i];
          if(label==3)  ss[0] << line[i]; // Nom
          if(label==14) ss[1] << line[i]; // Population 2010
          if(label==19) ss[2] << line[i]; // Longitude en degrés
          if(label==20) ss[3] << line[i]; // Latitude en degrés
        }
      }

      string::size_type sz;
      string my_nom = ss[0].str();
      int    my_pop = stoi(ss[1].str(),&sz);
      float  my_lon = stof(ss[2].str(),&sz);
      float  my_lat = stof(ss[3].str(),&sz);
      int    my_dep = stoi(ss[4].str(),&sz);

      if(my_pop >= popMin){
        villesNom[index] = my_nom;
        villesPop[index] = my_pop;
        villesLon[index] = my_lon;
        villesLat[index] = my_lat;
        villesDeparte[index] = my_dep;
        if(my_dep > departe_curr){
          beginDeparte[departe_count] = index;
          //cout << "beginDeparte[" << departe_count << "] : " << index << "\n"<<endl;
          departe_curr = my_dep;
          departe_count++;
        }
        index++;
      }
    }
    beginDeparte[departe_count] = nbVilles;
    //cout << "last beginDeparte[" << departe_count << "] : " << nbVilles+1 << "\n"<<endl;
    //cout<<" max departe_curr : "<<departe_curr<<endl;
    inputFile.close();
  }
  else
    cout << "   Impossible d'ouvrir le fichier." << endl;

//-----------------------------------------------------------------
//--- ECRITURE des villes de 'popMin' habitants ou plus
//-----------------------------------------------------------------

  cout << "== Écriture des villes de min. " << popMin << " habitants dans 'resuVilles.dat' ==" << endl;

  ofstream fileOut("resuVilles.dat");
  if (fileOut.is_open()) {
    for(int i=0; i<nbVilles; i++)
      fileOut << villesPop[i] << " "
              << villesLon[i] << " "
              << villesLat[i] << "\n";
    fileOut.close();
  }
  else
    cout << "   Impossible d'ouvrir le fichier." << endl;

  return;
}
