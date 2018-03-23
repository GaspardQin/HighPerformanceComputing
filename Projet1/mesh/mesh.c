#include <mesh.h>


int lnofa[4][3] = {{1,2,3},{2,3,0},{3,0,1},{0,1,2}};


Mesh * msh_init()
{
  Mesh *msh = malloc(sizeof(Mesh));
  if ( ! msh ) return NULL;
  
  msh->Dim    = 0;
  msh->NbrVer = 0;
  msh->NbrTri = 0;
  msh->NbrTet = 0;
  
  msh->Ver = NULL;
  msh->Tri = NULL;  
  msh->Tet = NULL; 
 
  
  msh->bb[0] = 0.0; /* xmin  */
  msh->bb[1] = 0.0; /* xmax  */
  msh->bb[2] = 0.0; /* ymin  */
  msh->bb[3] = 0.0; /* ymax  */
  msh->bb[4] = 0.0; /* zmin  */
  msh->bb[5] = 0.0; /* zmax  */ 
  
  return msh;
  
}  
  
  
Mesh * msh_read(char *file)
{
  char   InpFil[1024];
  float  bufFlt[3];
  double bufDbl[3];
  int    i,bufTet[5],bufFac[4];
  int    FilVer, ref; 
  
  int64_t fmsh = 0;
  
  if ( ! file ) return NULL;
  
  Mesh * msh = msh_init();
    
  //--- set file name 
  strcpy(InpFil,file);
  if ( strstr(InpFil,".mesh") ) {
    if ( !(fmsh = GmfOpenMesh(InpFil,GmfRead,&FilVer,&msh->Dim)) ) {
      return NULL;
    }    
  }
  else {
    strcat(InpFil,".meshb");
    if ( !(fmsh = GmfOpenMesh(InpFil,GmfRead,&FilVer,&msh->Dim)) ) {
      strcpy(InpFil,file);
      strcat(InpFil,".mesh");
      if ( !(fmsh = GmfOpenMesh(InpFil,GmfRead,&FilVer,&msh->Dim)) ) {
        return NULL;
      }    
    } 
  }
  
  printf(" File %s opened Dimension %d Version %d \n",InpFil,msh->Dim, FilVer);
  
  msh->NbrVer = GmfStatKwd(fmsh, GmfVertices);
  msh->NbrTet = GmfStatKwd(fmsh, GmfTetrahedra);
  msh->NbrTri = GmfStatKwd(fmsh, GmfTriangles);
  
  /* allocate arrays */
  msh->Ver = calloc( (msh->NbrVer+1), sizeof(Vertex)       );
  msh->Tri = calloc( (msh->NbrTri+1), sizeof(Triangle)     );  
  msh->Tet = calloc( (msh->NbrTet+1), sizeof(Tetrahedron)  ); 
  
  
   GmfGotoKwd(fmsh, GmfVertices);
   if ( msh->Dim == 2 ) {
     if ( FilVer == GmfFloat ) {		// read 32 bits float
       for (i=1; i<=msh->NbrVer; ++i) {
         GmfGetLin(fmsh, GmfVertices, &bufFlt[0], &bufFlt[1], &ref);
         msh->Ver[i].Crd[0] = (double)bufFlt[0];
         msh->Ver[i].Crd[1] = (double)bufFlt[1];
         msh->Ver[i].Crd[2] = 0.0;
       }
     }
     else  {	// read 64 bits float
       for (i=1; i<=msh->NbrVer; ++i) {
         GmfGetLin(fmsh, GmfVertices, &bufDbl[0], &bufDbl[1], &ref);
         msh->Ver[i].Crd[0] = bufDbl[0];
         msh->Ver[i].Crd[1] = bufDbl[1];
         msh->Ver[i].Crd[2] = 0.0;
       }  
     }
   }
   else {
     if ( FilVer == GmfFloat ) {		// read 32 bits float
       for (i=1; i<=msh->NbrVer; ++i) {
         GmfGetLin(fmsh, GmfVertices, &bufFlt[0], &bufFlt[1], &bufFlt[2], &ref);
         msh->Ver[i].Crd[0] = (double)bufFlt[0];
         msh->Ver[i].Crd[1] = (double)bufFlt[1];
         msh->Ver[i].Crd[2] = (double)bufFlt[2];
       }
     }
     else  {	// read 64 bits float
       for (i=1; i<=msh->NbrVer; ++i) {
         GmfGetLin(fmsh, GmfVertices, &bufDbl[0], &bufDbl[1], &bufDbl[2], &ref);
         msh->Ver[i].Crd[0] = bufDbl[0];
         msh->Ver[i].Crd[1] = bufDbl[1];
         msh->Ver[i].Crd[2] = bufDbl[2];
       }  
     }
   }
   
  
   //--- read tetrahedra 
  GmfGotoKwd(fmsh, GmfTetrahedra);
  for (i=1; i<=msh->NbrTet; ++i) {
    GmfGetLin(fmsh, GmfTetrahedra, &bufTet[0], &bufTet[1], &bufTet[2], &bufTet[3], &bufTet[4]);
    msh->Tet[i].Ver[0]    = bufTet[0];
    msh->Tet[i].Ver[1]    = bufTet[1];
    msh->Tet[i].Ver[2]    = bufTet[2];
    msh->Tet[i].Ver[3]    = bufTet[3];
    msh->Tet[i].Ref       = bufTet[4];
  }
  
  GmfGotoKwd(fmsh, GmfTriangles);
  for (i=1; i<=msh->NbrTri; ++i) {
    GmfGetLin(fmsh, GmfTriangles, &bufFac[0], &bufFac[1], &bufFac[2], &bufFac[3]);
    msh->Tri[i].Ver[0]    = bufFac[0];
    msh->Tri[i].Ver[1]    = bufFac[1];
    msh->Tri[i].Ver[2]    = bufFac[2];
    msh->Tri[i].Ref       = bufFac[3];
  }
  
  GmfCloseMesh(fmsh);
  
  return msh;
  
}


int compar_vertex(const void *a, const void *b)
{
  Vertex *va = (Vertex *) a;
  Vertex *vb = (Vertex *) b;
  return ( vb->icrit - va->icrit );
}

int compar_triangle(const void *a, const void *b)
{
  Triangle *va = (Triangle *) a;
  Triangle *vb = (Triangle *) b;
  return ( vb->icrit - va->icrit );
}

int compar_tetrahedron(const void *a, const void *b)
{
  Tetrahedron *va = (Tetrahedron *) a;
  Tetrahedron *vb = (Tetrahedron *) b;
  return ( vb->icrit - va->icrit );
}


int    msh_reorder(Mesh *msh)
{
  
  int iTet, iTri, iVer;
  
  if ( ! msh            ) return 0;
  if ( msh->NbrVer <= 0 ) return 0;
  
  /* compute bonding box */
  unsigned int x_index, y_index, z_index;
  unsigned long long int icrit_temp;
  for(iVer=1; iVer<=msh->NbrVer; iVer++) {
    /* todo msh->bb : used to compute the Z-curve index */
    x_index = msh->Ver[iVer].Crd[0];
    y_index = msh->Ver[iVer].Crd[1];
    z_index = msh->Ver[iVer].Crd[2];
    
    if ((x_index >> 21) > 0)
        printf("x out of index"); 
    
    if ((y_index >> 21) > 0)
        printf("y out of index"); 
    
    if ((z_index >> 21) > 0)
        printf("z out of index"); 
    
//http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/


    msh->Ver[iVer].icrit  = icrit_temp;
    msh->Ver[iVer].idxOld = iVer;
  }
  /*
  for(iVer=1; iVer<=msh->NbrVer; iVer++) {
    msh->Ver[iVer].icrit  = rand();   // change the randon  by Z  order 
    msh->Ver[iVer].idxNew = iVer;
    msh->Ver[iVer].idxOld = iVer;
  }
  */
  
  
  
  qsort(&msh->Ver[1],msh->NbrVer,sizeof(Vertex), compar_vertex); 

  
  /* update idxNew for vertices */
  for(iVer=1; iVer<=msh->NbrVer; iVer++) {
    msh->Ver[iVer].idxNew = iVer;
  }
  
  /* re-assign triangles and tets ids */
  
  
  /* sort triangles */
  for(iTri=1; iTri<=msh->NbrTri; iTri++) {
    msh->Tri[iTri].icrit  = rand();  
  }
  qsort(&msh->Tri[1],msh->NbrTri,sizeof(Triangle), compar_triangle);
  
  /* sort tetrahedra */
  for(iTet=1; iTet<=msh->NbrTet; iTet++) {
    msh->Tet[iTet].icrit  = rand();  
  }
  qsort(&msh->Tet[1],msh->NbrTet,sizeof(Tetrahedron), compar_tetrahedron);
  
  return 1;
}



int    msh_write(Mesh *msh, char *file)
{
   int iVer, iTri, iTfr, iTet;
   int FilVer = 2;
  
   if ( ! msh  ) return 0;
   if ( ! file ) return 0;
   
   int64_t fmsh = GmfOpenMesh(file, GmfWrite, FilVer, msh->Dim);
   if ( fmsh <=  0 ) {
     printf("  ## ERROR: CANNOT CREATE FILE \n");
     return 0;
   }

   GmfSetKwd(fmsh, GmfVertices,   msh->NbrVer);
   if ( msh->Dim == 3 ) {
     for ( iVer=1; iVer<=msh->NbrVer; iVer++){
       GmfSetLin(fmsh, GmfVertices, msh->Ver[iVer].Crd[0],msh->Ver[iVer].Crd[1],msh->Ver[iVer].Crd[2],0); 
     }
   }
   else {
     for ( iVer=1; iVer<=msh->NbrVer; iVer++){
       GmfSetLin(fmsh, GmfVertices, msh->Ver[iVer].Crd[0],msh->Ver[iVer].Crd[1],0); 
     }
   }
   
   GmfSetKwd(fmsh, GmfTriangles, msh->NbrTri);
   for ( iTfr=1; iTfr<=msh->NbrTri; iTfr++)  
     GmfSetLin(fmsh, GmfTriangles, msh->Tri[iTfr].Ver[0], msh->Tri[iTfr].Ver[1], msh->Tri[iTfr].Ver[2], msh->Tri[iTfr].Ref);  
   
   GmfSetKwd(fmsh, GmfTetrahedra, msh->NbrTet);
   for ( iTet=1; iTet<=msh->NbrTet; iTet++) 
     GmfSetLin(fmsh, GmfTetrahedra, msh->Tet[iTet].Ver[0], msh->Tet[iTet].Ver[1],msh->Tet[iTet].Ver[2], msh->Tet[iTet].Ver[3], 0);
   
   GmfCloseMesh(fmsh);
   
   return 1;
   
}






int  msh_neighborsQ2(Mesh *msh)
{
  int iTet, iFac, jTet, jFac, ip1, ip2 ,ip3 , jp1, jp2, jp3;
  
  if ( ! msh ) return 0;
  
  for(iTet=1; iTet<=msh->NbrTet; iTet++) {
    for(iFac=0; iFac<4; iFac++) {
      ip1 = msh->Tet[iTet].Ver[lnofa[iFac][0]];
      ip2 = msh->Tet[iTet].Ver[lnofa[iFac][1]];
      ip3 = msh->Tet[iTet].Ver[lnofa[iFac][2]];
      /* find the Tet different from iTet that has ip1, ip2, ip2 as vertices */
      for(jTet=1; jTet<=msh->NbrTet; jTet++) {
        if ( iTet == jTet ) continue;
        for(jFac=0; jFac<4; jFac++) {
          jp1 = msh->Tet[jTet].Ver[lnofa[jFac][0]];
          jp2 = msh->Tet[jTet].Ver[lnofa[jFac][1]];
          jp3 = msh->Tet[jTet].Ver[lnofa[jFac][2]];
          /* compare the 6 points */
        }
      }
      
    }
  }
  
  return 1;
}   



int  msh_neighbors(Mesh *msh)
{
  int iTet, iFac, ip1, ip2 ,ip3 ;
  
  if ( ! msh ) return 0;
  
  /* initialize HashTable */
  
  for(iTet=1; iTet<=msh->NbrTet; iTet++) {
    for(iFac=0; iFac<4; iFac++) {
      ip1 = msh->Tet[iTet].Ver[lnofa[iFac][0]];
      ip2 = msh->Tet[iTet].Ver[lnofa[iFac][1]];
      ip3 = msh->Tet[iTet].Ver[lnofa[iFac][2]];
      /* compute the key : ip1+ip2+ip3   */
      /* do we have objects as that key   hash_find () */
      /*  if yes ===> look among objects and potentially update Voi */
      /*  if no  ===> add to hash table.  hash_add()   */
    }
  }
  return 1;
}   



