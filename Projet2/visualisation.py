# python visualisation.py

from matplotlib import pyplot
from numpy import loadtxt

villes = loadtxt("resuVilles.dat", dtype=float, delimiter=" ")
pyplot.scatter(villes[:,1], villes[:,2], s=villes[:,0]/1000, c=villes[:,0], alpha=0.5);

graphe = loadtxt("resuGraphe.dat", dtype=int)
for x in xrange(graphe.shape[0]):
  arete = [graphe[x,0], graphe[x,1]]
  pyplot.plot(villes[arete,1], villes[arete,2], 'b')

pyplot.xlabel('Longitude', size=16)
pyplot.ylabel('Latitude', size=16)

# Pour enregistrer dans un fichier PNG :
pyplot.savefig('resultat.png')

# Pour affichier :
pyplot.show()
