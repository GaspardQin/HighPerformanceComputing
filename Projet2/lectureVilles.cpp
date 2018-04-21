#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "lectureVilles.h"

using namespace std;

void lectureVilles(int popMin, string* &villesNom, int* &villesPop, float* &villesLon, float* &villesLat, int &nbVilles){

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
    //villesLon = new double[nbVilles];
    villesLon = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
    //villesLat = new double[nbVilles];
    villesLat = (float*)_mm_malloc(nbVilles * sizeof(float),VEC_ALIGN);
    // Lecture/Sauvegarde des données

    int index=0;
    inputFile.clear();
    inputFile.seekg(0, ios::beg);
    while ( getline(inputFile,line) ) {

      int label=0;
      stringstream ss[4];
      for(int i=0; i<line.size(); i++){
        if(line[i] == ',')
          label++;
        else{
          if(label==3)  ss[0] << line[i]; // Nom
          if(label==14) ss[1] << line[i]; // Population 2010
          if(label==19) ss[2] << line[i]; // Longitude en degrés
          if(label==20) ss[3] << line[i]; // Latitude en degrés
        }
      }

      string::size_type sz;
      string my_nom = ss[0].str();
      int    my_pop = stoi(ss[1].str(),&sz);
      double  my_lon = stof(ss[2].str(),&sz);
      double  my_lat = stof(ss[3].str(),&sz);

      if(my_pop >= popMin){
        villesNom[index] = my_nom;
        villesPop[index] = my_pop;
        villesLon[index] = my_lon;
        villesLat[index] = my_lat;
        index++;
      }
    }

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
