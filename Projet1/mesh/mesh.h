/* 


  Mesh Structures that will be used in the project 
  
  
  Provided functions : 
       
  

*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <libmesh6.h>
#include <lplib3.h>



typedef double double3[3];
typedef int    int4[4]; 
typedef int    int3[3];
typedef int    int1[1];
typedef unsigned long long int u64;


/* 
  
  A simple mesh structure to hold the geomety/connectivity 
  Note that 2D mesh are stored as 3D Mesh, z-coordinate will be set to zero

  Provided function msh_init, msh_read, msh_write, msh_check
  The mesh files are based on the meshb format (https://github.com/LoicMarechal/libMeshb), can be viewed with Medit, see README to install it. 

  The following functions has to be implemented  : 
    msh_bondingbox  : compute the bounding box of mesh
    msh_reorder     : reorder an input mesh according to Z-ordering 
    msh_reorder     : simple smoothing algorithm of volume points   
    msh_neighbors   :  build the set of Tets/Tris surrounding elements, efficient implementation with Hash Tab
    msh_neighborsQ2 : a quadratic setting of the neigbhoring structures 

 

*/

typedef struct mesh_vertex
{  
  double Crd[3];
  
  unsigned long long int icrit; /* sorting creteria, to be used with qsort  */
  int idxNew;          /* new if after sort */
  int idxOld;          /* initial id  */ 
  
} Vertex; 

typedef struct mesh_triangle
{
  int Ver[3]; /* index of the 3-vertex  */
  int Voi[3]; /* neigbhoring structures */
  int Ref;
  
  unsigned long long int icrit; /* sorting creteria, to be used with qsort  */
  
} Triangle;

typedef struct mesh_tetrahedron
{
  int Ver[4]; /* vertices indices */
  int Voi[4]; /* id of the neigbhoring faces */
  int Ref;
  
  unsigned long long int icrit; /* sorting creteria, to be used with qsort  */
  
} Tetrahedron;

typedef struct t_mesh
{
  int Dim;
  int NbrVer, NbrTri, NbrTet;
  
  Vertex      *Ver;
  Triangle    *Tri;
  Tetrahedron *Tet;
  
  /* bounding box */
  double bb[6];
  
 
} Mesh; 


/* Provided functions */
Mesh * msh_init();
Mesh * msh_read(char *file);
int    msh_write(Mesh *msh, char *file); 



/* functions to be implemented  */
int    msh_boundingbox(Mesh *msh);         /* compute the bouding box of the mesh                         */
int    msh_reorder(Mesh *msh);             /* perform a mesh using morton curve/octree-based              */
int    msh_smooth(Mesh *msh, int nbrStep); /* a simple mesh smoohting algorithm                           */
int    msh_neighbors(Mesh *msh);           /* build TetVois or TriVois with a hash table                  */
int    msh_neighborsQ2(Mesh *msh);         /* biuld TetVois with the naive quadratic approach             */
  

/* a provided simple hash table data structure */
typedef int int6[6];

typedef struct mesh_hash_table
{
  int  SizHead;   /* Maxmimum entries, the key is in [0,SizHead-1]*/
  int  NbrObj;    /* Number of object in the hash tables */
  int  NbrMaxObj; /* Maximum of object that can be store in the hash tab */ 
  int  *Head ;  /* Head[key%(SizHead)] = link to the first object having this key  in the LstObj list */
  int6 *LstObj; /* List of objects in the Hash Tab */
  
  /* LstObj[id][0:2] = ip1-ip2-ip3, the 3 points defining the face  */
  /* LstObj[id][3:4] = iTet1,iTet2, the Two neighboring tets having ip1-ip2-ip3 as points */
  /* LstObj[id][5]   = idnxt the link to the next element in collision, if = 0 last element of the list */
  
} HashTable;

/* Implementing the following function should be necessart */
/* HasTable * hash_init(int SizHead, int NbrMaxObj); ==> allocate Head, LstObj */
/* int hash_find(HashTable *hsh, int ip1, int ip2, int ip3);  return the id found (in LstObj ), if 0 the object is not in the list */
/* int hash_add(HashTable, *hsh, int ip1, int ip2m int ip3, int iTet) ===> add this entry in the hash tab */



