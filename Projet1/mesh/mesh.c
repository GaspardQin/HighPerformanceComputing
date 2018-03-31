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

int double_compare( const void* a, const void* b)
{
     double _a = * ( (double*) a );
     double _b = * ( (double*) b );

     if ( _a == _b ) return 0;
     else if ( _a < _b ) return -1;
     else return 1;
}
int quick_find( double * array,int start, int end, double value){
  int middle_index = (start + end) /2;

  if(array[middle_index] > value){
    quick_find(array, start, middle_index - 1, value);
  }
  else{
    if(array[middle_index] < value)
      quick_find(array, middle_index + 1, end, value);
    else{
      if (array[middle_index] == value)
        return middle_index;
      else
        printf("can't find index\n" );
    }
  }

}
int    msh_reorder(Mesh *msh)
{

  int iTet, iTri, iVer;
  int i;
  if ( ! msh            ) return 0;
  if ( msh->NbrVer <= 0 ) return 0;

  /* compute bonding box */
  unsigned int x_index, y_index, z_index;
  unsigned long long int icrit_temp;
  double porp = 1;
  double max_var[3]={0};
  double min_var[3]={0};
  double *x_list, *y_list, *z_list;
  x_list = calloc(msh->NbrVer, sizeof(double));
  y_list = calloc(msh->NbrVer, sizeof(double));
  z_list = calloc(msh->NbrVer, sizeof(double));
  for(iVer=1; iVer<=msh->NbrVer; iVer++) {
    /* todo msh->bb : used to compute the Z-curve index */
    x_list[iVer-1] = msh->Ver[iVer].Crd[0];
    y_list[iVer-1] = msh->Ver[iVer].Crd[1];
    z_list[iVer-1] = msh->Ver[iVer].Crd[2];

    for(i = 0; i < 3; i++){

      if(msh->Ver[iVer].Crd[i] > max_var[i])
        max_var[i] = msh->Ver[iVer].Crd[i];

      if(msh->Ver[iVer].Crd[i] < min_var[i])
        min_var[i] = msh->Ver[iVer].Crd[i];
    }
  }
  qsort(x_list, msh->NbrVer, sizeof(double), double_compare);
  qsort(y_list, msh->NbrVer, sizeof(double), double_compare);
  qsort(z_list, msh->NbrVer, sizeof(double), double_compare);

  msh->bb[0] = min_var[0];
  msh->bb[1] = max_var[0];
  msh->bb[2] = min_var[1];
  msh->bb[3] = max_var[1];
  msh->bb[4] = min_var[2];
  msh->bb[5] = max_var[2];
  printf("finished bb\n");
  for(i=0;i<6;i++){
    printf("bb %f\n", msh->bb[i]);
  }


  int* x_index_debug, *y_index_debug, *z_index_debug;
  x_index_debug = calloc((msh->NbrVer), sizeof(int)  );
  y_index_debug = calloc((msh->NbrVer), sizeof(int)  );
  z_index_debug = calloc((msh->NbrVer), sizeof(int)  );
  for(iVer=1; iVer<=msh->NbrVer; iVer++) {
    /* todo msh->bb : used to compute the Z-curve index */
    x_index = quick_find(x_list, 0, msh->NbrVer, msh->Ver[iVer].Crd[0]);
    y_index = quick_find(y_list, 0, msh->NbrVer, msh->Ver[iVer].Crd[1]);
    z_index = quick_find(z_list, 0, msh->NbrVer, msh->Ver[iVer].Crd[2]);

    x_index_debug[iVer-1] = x_index;
    y_index_debug[iVer-1] = y_index;
    z_index_debug[iVer-1] = z_index;
    if ((x_index >> 21) > 0)
        printf("x out of index vertical\n");

    if ((y_index >> 21) > 0)
        printf("y out of index vertical\n");

    if ((z_index >> 21) > 0)
        printf("z out of index vertical\n");

//http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
    icrit_temp = 0;

    for (i = 0; i < (sizeof(unsigned long long int)* CHAR_BIT)/3; ++i) {
      icrit_temp |= ((x_index & ((unsigned long long int)1 << i)) << 2*i) | ((y_index & ((unsigned long long int)1 << i)) << (2*i + 1)) | ((z_index & ((unsigned long long int)1 << i)) << (2*i + 2));
    }

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

  int int_compare( const void* a, const void* b)
  {
       int int_a = * ( (int*) a );
       int int_b = * ( (int*) b );

       if ( int_a == int_b ) return 0;
       else if ( int_a < int_b ) return -1;
       else return 1;
  }
  qsort(x_index_debug,msh->NbrVer,sizeof(int),int_compare);
  qsort(y_index_debug,msh->NbrVer,sizeof(int),int_compare);
  qsort(z_index_debug,msh->NbrVer,sizeof(int),int_compare);

  for(iVer = 0;iVer < msh->NbrVer; iVer++){
    printf("x_index: %d, y_index: %d, z_index: %d\n", x_index_debug[iVer],y_index_debug[iVer],z_index_debug[iVer] );
  }

  qsort(&msh->Ver[1],msh->NbrVer,sizeof(Vertex), compar_vertex);

  int *newIndex;
  newIndex = calloc( (msh->NbrVer+1), sizeof(int));

  /* update idxNew for vertices */
  for(iVer=1; iVer<=msh->NbrVer; iVer++) {
    msh->Ver[iVer].idxNew = iVer;
    newIndex[msh->Ver[iVer].idxOld] = iVer;
  }

  /* re-assign triangles and tets ids */

  /* sort triangles */
  int j; int temp_index;

  for(iTri=1; iTri<=msh->NbrTri; iTri++) {
    for(j =0; j < 3; j++){
      temp_index = msh->Tri[iTri].Ver[j];
      msh->Tri[iTri].Ver[j] = newIndex[temp_index];
    }
  }

  /* sort tetrahedra */

  for(iTet=1; iTet<=msh->NbrTet; iTet++) {
    for(j =0; j < 4; j++){
      temp_index = msh->Tet[iTet].Ver[j];
      msh->Tet[iTet].Ver[j] = newIndex[temp_index];
    }
  }
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
