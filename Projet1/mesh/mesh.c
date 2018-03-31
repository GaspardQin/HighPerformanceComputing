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



int compar_vertex( const void* a, const void* b)
{
  Vertex *va = (Vertex *) a;
  Vertex *vb = (Vertex *) b;
  if ( vb->icrit == va->icrit ) return 0;
  else {
    if ( vb->icrit > va->icrit ) return -1;
    else return 1;
  }
}
int compar_triangle(const void *a, const void *b)
{
  Triangle *va = (Triangle *) a;
  Triangle *vb = (Triangle *) b;
  if ( vb->icrit == va->icrit ) return 0;
  else {
    if ( vb->icrit > va->icrit ) return -1;
    else return 1;
  }
}

int compar_tetrahedron(const void *a, const void *b)
{
  Tetrahedron *va = (Tetrahedron *) a;
  Tetrahedron *vb = (Tetrahedron *) b;
  if ( vb->icrit == va->icrit ) return 0;
  else {
    if ( vb->icrit > va->icrit ) return -1;
    else return 1;
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
  for(iVer=1; iVer<=msh->NbrVer; iVer++) {
    /* todo msh->bb : used to compute the Z-curve index */
    x_index = (msh->Ver[iVer].Crd[0] - msh->bb[0])/porp;
    y_index = (msh->Ver[iVer].Crd[1] - msh->bb[2])/porp;
    z_index = (msh->Ver[iVer].Crd[2] - msh->bb[4])/porp;
    for(i = 0; i < 3; i++){
      if(msh->Ver[iVer].Crd[i] > max_var[i])
        max_var[i] = msh->Ver[iVer].Crd[i];

      if(msh->Ver[iVer].Crd[i] < min_var[i])
        min_var[i] = msh->Ver[iVer].Crd[i];
    }
  }
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
  while(1){
    x_index = (unsigned int)((msh->bb[1] - msh->bb[0])/porp);
    y_index = (unsigned int)((msh->bb[3] - msh->bb[2])/porp);
    z_index = (unsigned int)((msh->bb[5] - msh->bb[4])/porp);
    //printf("x is %f\n", (msh->bb[1] - msh->bb[0])/porp);
    //printf("y is %u\n", y_index);
    //printf("z is %u\n", z_index);
    if (((x_index | y_index | z_index) >> 21) > 0)
      porp *= 2;
    else {
      if(((x_index | y_index | z_index) >> 20) <= 0)
          porp /= 2;
      else{
        printf("porp is %f\n", porp);
        printf("x is %u\n", x_index);
        printf("y is %u\n", y_index);
        printf("z is %u\n", z_index);
        //return;
        break;
      }
    }
  }

  int* x_index_debug, *y_index_debug, *z_index_debug;
  x_index_debug = calloc((msh->NbrVer), sizeof(int)  );
  y_index_debug = calloc((msh->NbrVer), sizeof(int)  );
  z_index_debug = calloc((msh->NbrVer), sizeof(int)  );
  for(iVer=1; iVer<=msh->NbrVer; iVer++) {
    /* todo msh->bb : used to compute the Z-curve index */
    x_index = (unsigned int)((msh->Ver[iVer].Crd[0] - msh->bb[0])/porp);
    y_index = (unsigned int)((msh->Ver[iVer].Crd[1] - msh->bb[2])/porp);
    z_index = (unsigned int)((msh->Ver[iVer].Crd[2] - msh->bb[4])/porp);
    //printf("x_prop, y_prop, z_prop : %d, %d, %d\n",x_index,y_index,z_index );
/*
    x_index_debug[iVer-1] = x_index;
    y_index_debug[iVer-1] = y_index;
    z_index_debug[iVer-1] = z_index;
*/
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
/*
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
*/

  qsort(&msh->Ver[1],msh->NbrVer,sizeof(Vertex), compar_vertex);
  /*
  for(iVer = 1;iVer < msh->NbrVer; iVer++){
    printf("x_index: %d, y_index: %d, z_index: %d\n", x_index_debug[iVer],y_index_debug[iVer],z_index_debug[iVer] );
    printf("icrit: %llu\n", msh->Ver[iVer].icrit );
  }
*/
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

  for(iTri=1; iTri<=msh->NbrTri; iTri++) {
    /* todo msh->bb : used to compute the Z-curve index */
    //printf("index of tri neigbhors, %d, %d, %d\n",msh->Tri[iTri].Voi[0],msh->Tri[iTri].Voi[1],msh->Tri[iTri].Voi[2]);
    x_index = 0;
    y_index = 0;
    z_index = 0;
    for(i=0;i<3;i++){
      int vertex_index = msh->Tri[iTri].Ver[i];
      x_index += (unsigned int)((msh->Ver[vertex_index].Crd[0] - msh->bb[0])/porp);
      y_index += (unsigned int)((msh->Ver[vertex_index].Crd[1] - msh->bb[2])/porp);
      z_index += (unsigned int)((msh->Ver[vertex_index].Crd[2] - msh->bb[4])/porp);
    }
    x_index = x_index/3;
    y_index = y_index/3;
    z_index = z_index/3;
    if ((x_index >> 21) > 0)
        printf("triangles: x out of index vertical\n");

    if ((y_index >> 21) > 0)
        printf("triangles: y out of index vertical\n");

    if ((z_index >> 21) > 0)
        printf("triangles: z out of index vertical\n");

//http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
    icrit_temp = 0;

    for (i = 0; i < (sizeof(unsigned long long int)* CHAR_BIT)/3; ++i) {
      icrit_temp |= ((x_index & ((unsigned long long int)1 << i)) << 2*i) | ((y_index & ((unsigned long long int)1 << i)) << (2*i + 1)) | ((z_index & ((unsigned long long int)1 << i)) << (2*i + 2));
    }

    msh->Tri[iTri].icrit  = icrit_temp;
    msh->Tri[iTri].idxOld = iTri;
  }
  qsort(&msh->Tri[1],msh->NbrTri,sizeof(Triangle), compar_triangle);

  int *newIndexTri;
  newIndexTri = calloc( (msh->NbrTri+1), sizeof(int));

  /* update idxNew for Tri */
  for(iTri=1; iTri<=msh->NbrTri; iTri++) {
    newIndexTri[msh->Tri[iTri].idxOld] = iTri;
  }
  for(iTri = 1; iTri<=msh->NbrTri; iTri++){
    for(j = 0; j<3; j++){
      msh->Tri[iTri].Voi[j] = newIndexTri[msh->Tri[iTri].Voi[j]];
    }
  }

  /* sort tetrahedra */

  for(iTet=1; iTet<=msh->NbrTet; iTet++) {
    for(j =0; j < 4; j++){
      temp_index = msh->Tet[iTet].Ver[j];
      msh->Tet[iTet].Ver[j] = newIndex[temp_index];
    }
  }


  for(iTet=1; iTet<=msh->NbrTet; iTet++) {
    /* todo msh->bb : used to compute the Z-curve index */
    x_index = 0;
    y_index = 0;
    z_index = 0;
    for(i=0;i<4;i++){
      int vertex_index = msh->Tet[iTet].Ver[i];
      x_index += (unsigned int)((msh->Ver[vertex_index].Crd[0] - msh->bb[0])/porp);
      y_index += (unsigned int)((msh->Ver[vertex_index].Crd[1] - msh->bb[2])/porp);
      z_index += (unsigned int)((msh->Ver[vertex_index].Crd[2] - msh->bb[4])/porp);
    }
    x_index = x_index/4;
    y_index = y_index/4;
    z_index = z_index/4;
    if ((x_index >> 21) > 0)
        printf("Tetrahedron: x out of index, %d \n", x_index);

    if ((y_index >> 21) > 0)
        printf("Tetrahedron: y out of index %d \n", y_index);

    if ((z_index >> 21) > 0)
        printf("Tetrahedron: z out of index %d \n", z_index);


//http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
    icrit_temp = 0;

    for (i = 0; i < (sizeof(unsigned long long int)* CHAR_BIT)/2; ++i) {
      icrit_temp |= ((x_index & ((unsigned long long int)1 << i)) << 2*i)
      |((y_index & ((unsigned long long int)1 << i)) << (2*i + 1))
      |((z_index & ((unsigned long long int)1 << i)) << (2*i + 2));
    }

    msh->Tet[iTet].icrit  = icrit_temp;
    msh->Tet[iTet].idxOld = iTri;
  }
  qsort(&msh->Tet[1],msh->NbrTet,sizeof(Tetrahedron), compar_tetrahedron);

  int *newIndexTet;
  newIndexTet = calloc( (msh->NbrTet+1), sizeof(int));

  /* update idxNew for Tri */
  for(iTet=1; iTet<=msh->NbrTet; iTet++) {
    newIndexTet[msh->Tet[iTet].idxOld] = iTet;
  }
  for(iTet = 1; iTet<=msh->NbrTet; iTet++){
    for(j = 0; j<4; j++){
      msh->Tet[iTet].Voi[j] = newIndexTet[msh->Tet[iTet].Voi[j]];
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
  int break_flag = 0;
  int j;
  int is_find_tri;
  int is_find_tet;
  if ( ! msh ) return 0;
//int lnofa[4][3] = {{1,2,3},{2,3,0},{3,0,1},{0,1,2}};
  for(iTet=1; iTet<=msh->NbrTet; iTet++) {
    for(iFac=0; iFac<4; iFac++) {
      ip1 = msh->Tet[iTet].Ver[lnofa[iFac][0]];
      ip2 = msh->Tet[iTet].Ver[lnofa[iFac][1]];
      ip3 = msh->Tet[iTet].Ver[lnofa[iFac][2]];
      /* find the Tet different from iTet that has ip1, ip2, ip2 as vertices */
      break_flag = 0;

      for(j=1; j<=msh->NbrTet + 40; j++) {
        if( j <= 40 )
          jTet = iTet - 20 + j;
        else
            jTet = j - 40;
        if ( iTet == jTet ) continue;
        if (break_flag == 1) break;
        for(jFac=0; jFac<4; jFac++) {
          jp1 = msh->Tet[jTet].Ver[lnofa[jFac][0]];
          jp2 = msh->Tet[jTet].Ver[lnofa[jFac][1]];
          jp3 = msh->Tet[jTet].Ver[lnofa[jFac][2]];
          /* compare the 6 points */
          if(ip1 == jp1 && ip2 == jp2 && ip3 == jp3){
            printf("find tet neighbors of %d : %d\n", iTet, jTet);
            msh->Tet[iTet].Voi[iFac] = jTet;
            break_flag = 1;
            break;
          }
        }
      }
    }
  }

//find neigbhors of triangles
int iTri, jTri; int iEdge, jEdge;
int lnofe[3][2] = {{0,1},{1,2},{2,0}};
for(iTri=1; iTri<=msh->NbrTri; iTri++) {
  for(iEdge=0; iEdge<3; iEdge++) {
    ip1 = msh->Tri[iTri].Ver[lnofe[iEdge][0]];
    ip2 = msh->Tri[iTri].Ver[lnofe[iEdge][1]];
    /* find the Tet different from iTet that has ip1, ip2, ip2 as vertices */
    break_flag = 0;
    for(jTri=1; jTri<=msh->NbrTet; jTri++) {
      if ( iTri == jTri ) continue;
      if (break_flag == 1) break;
      for(jEdge=0; jEdge<3; jFac++) {
        jp1 = msh->Tri[jTri].Ver[lnofe[jEdge][0]];
        jp2 = msh->Tri[jTri].Ver[lnofe[jEdge][1]];

        /* compare the 4 points */
        if(ip1 == jp1 && ip2 == jp2){
          printf("find tri neighbors of %d : %d\n", iTri, jTri);
          msh->Tri[iTri].Voi[iEdge] = jTri;
          break_flag = 1;
          break;
        }
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
