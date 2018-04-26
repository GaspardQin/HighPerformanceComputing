 # python visualisation.py
import sys
import os
from matplotlib import pyplot
from numpy import loadtxt, average, size
import numpy as np
def visualiser(villes, graphe, save_fig_name, mode, isShowWeight):
    fig = pyplot.figure()
    ax = fig.add_subplot(111)

    if mode == 'showResult':
        ax.scatter(villes[:,1], villes[:,2], s=villes[:,0]/1000, c=villes[:,0], alpha=0.5);
    else:
        #print(graphe, villes)
        colors = np.array( [ 'b', 'r'])
        ax.scatter(villes[:,1], villes[:,2], s=villes[:,0]/1000, c=colors[graphe[:,3].astype(int)], alpha=0.5);
    bbox_props = dict(boxstyle="round", fc="w", ec="0.5", alpha=0.9)

    for x in range(graphe.shape[0]):
      #if graphe[x,2] == 0.0:
    #      continue
      arete = [int(graphe[x,0]), int(graphe[x,1])]

      #print(villes)
      ax.plot(villes[arete,1], villes[arete,2], 'b')
      #print(weight)
     # print(villes[arete,1], villes[arete,2])
      if isShowWeight == 1:
         weight = graphe[x,2]
         ax.text(average(villes[arete,1]), average(villes[arete,2]), str(weight), ha="center", va="center", size=10,
            bbox=bbox_props)

    pyplot.xlabel('Longitude', size=16)
    pyplot.ylabel('Latitude', size=16)

    # Pour enregistrer dans un fichier PNG :
    pyplot.savefig(save_fig_name)

    # Pour affichier :
    if mode == 'showResult':
        pyplot.show()
    else:
        pyplot.close()
def showResult(resuGraphe_path,resuVilles_path,isShowWeight):
    villes = loadtxt(resuVilles_path, dtype=float, delimiter=" ")
    graphe = loadtxt(resuGraphe_path, dtype=float)
    visualiser(villes, graphe, 'showResult.png', 'showResult',isShowWeight)

def animation(resuGraphe_folder, resuVilles_path,isShowWeight):
    save_path = resuGraphe_folder + "/../step_anim"
    if not os.path.exists(save_path):
        os.makedirs(save_path)

    villes = loadtxt(resuVilles_path, dtype=float, delimiter=" ")
    for root, dirs, files in os.walk(resuGraphe_folder):
        for name in files:
            graphe = loadtxt(resuGraphe_folder + '/' + name, dtype=float)
            save_name = save_path + '/' + name.split('.')[0] + 'png'
            visualiser(villes, graphe, save_name, 'animation',isShowWeight)

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print 'Usage: python visualisation.py [path to resuGraphe.dat] [path to resuVilles.dat] [0 or 1, is show weight]'
        exit(1)

    resuGraphe_path = sys.argv[1]
    resuVilles_path = sys.argv[2]
    isShowWeight = int(sys.argv[3])
    if os.path.isfile(resuGraphe_path):
        showResult(resuGraphe_path, resuVilles_path, isShowWeight)
    else:
        print("processing each step's data")
        animation(resuGraphe_path, resuVilles_path, isShowWeight)