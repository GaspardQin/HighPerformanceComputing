#include <mesh.h>
#include <math.h>

int lnofa[4][3] = {{1,2,3},{2,3,0},{3,0,1},{0,1,2}};
HashTable* hash_init(int SizeHead, int NbrMaxObj){
  HashTable* ht = (HashTable*)calloc(1,sizeof(HashTable));
  ht->SizeHead = SizeHead;
  ht->NbrMaxObj = NbrMaxObj;
  //ht->Head = (int*)calloc(SizeHead,sizeof(int));
  ht->LstObj = (int*)calloc(NbrMaxObj, sizeof(int6));
  ht->NbrObj = ht->SizeHead;
  int i=0;
  return ht;
}

int is_equal(int *a, int b1, int b2, int b3){
  //a[0:2]
  int i =0;
  int count =0;
  for(i=0;i<3;i++){
    if(b1 == a[i]) count++;
    if(b2 == a[i]) count++;
    if(b3 == a[i]) count++;
  }
  if(count == 3) return 1;
  else return 0;
}

int hash_find(HashTable *hsh, int ip1, int ip2, int ip3, int iTet, int debug_switch){
  //return the id found (in LstObj ), if -1 the object is not in the list
  int key = (3*ip1 + 5*ip2 + 7*ip3);
  //printf("key: %d\n", key);
  int head = key%hsh->SizeHead;
  int old_head;
  while(1){
    if(is_equal(hsh->LstObj[head],ip1,ip2,ip3)){
      if(iTet >=0)
         hsh->LstObj[head][4] = iTet;
      return head;
    }

    else{
      old_head = head;
      head = hsh->LstObj[head][5];
      if(head == 0 ) break;
    //  if(old_head > head)
        //printf("detect loop ! head: %d\n", head);
      if(debug_switch==1)
        printf("head: %d\n", head);
    }

  }
  return -1;

}
void hash_add(HashTable *hsh, int ip1, int ip2, int ip3, int iTet1) //===> add this entry in the hash tab
{
  /* LstObj[id][0:2] = ip1-ip2-ip3, the 3 points defining the face  */
  /* LstObj[id][3:4] = iTet1,iTet2, the Two neighboring tets having ip1-ip2-ip3 as points */
  /* LstObj[id][5]   = idnxt the link to the next element in collision, if = 0 last element of the list */

  int key = (3*ip1 + 5*ip2 + 7*ip3) ;
  int head = key%hsh->SizeHead;
  int write_index;
  if(hsh->LstObj[head][0] <=0){
    write_index = head;
  }
  else{
    while(hsh->LstObj[head][5]>0)
      head = hsh->LstObj[head][5];

    hsh->LstObj[head][5] = hsh->NbrObj+1;
    if(head>hsh->NbrObj){
      printf("write loop, head: %d\n", head);
      exit(0);
    }
    write_index = hsh->NbrObj+1;
    hsh->NbrObj ++;
  }
  //printf("NbrObj: %d, NbrMaxObj: %d\n", hsh->NbrObj, hsh->NbrMaxObj);


  hsh->LstObj[write_index][0] = ip1;
  hsh->LstObj[write_index][1] = ip2;
  hsh->LstObj[write_index][2] = ip3;
  hsh->LstObj[write_index][3] = iTet1;
}


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
/*
typedef struct trianges_tet_for_each_vert
{
  int Tri[4];
  char count;
  TriTetVert* add_on; //used when more than 4 triangles/tets in a single vertical;
} TriTetVert;

int insert_TriTetVert(TriTetVert* ttv, int index){

  while(ttv->count > 4){
    ttv->count ++;
    ttv= ttv->add_on;
  }
  if(ttv->count == 4){
    ttv->add_on = (TriTetVert*)malloc(sizeof(TriTetVert));
    ttv->count ++;
    ttv = ttv->add_on;
  }
  ttv->Tri[ttv->count] = index;
  ttv->count ++;

}
int getIndexTriTetVert(TriTetVert* ttv, int index, )
int is_not_in_voi_tri(Triangle* tri){
  if(!tri)
    printf("wrong address of tri\n" );
  if(tri->Voi[0] == 0)
    return 0;
  if(tri->Voi[1] == 0)
    return 1;
  if(tri->Voi[2] == 0)
    return 2;
  else
    return -1;
}
int is_neighbor_tri(Triangle* tri1, Tringle* tri2){
  int iTri, jTri; int iEdge, jEdge;
  int lnofe[3][2] = {{0,1},{1,2},{2,0}};
    for(iEdge=0; iEdge<3; iEdge++) {
      ip1 = msh->Tri[iTri].Ver[lnofe[iEdge][0]];
      ip2 = msh->Tri[iTri].Ver[lnofe[iEdge][1]];
      // find the Tet different from iTet that has ip1, ip2, ip2 as vertices
      for(jEdge=0; jEdge<3; jFac++) {
        jp1 = msh->Tri[jTri].Ver[lnofe[jEdge][0]];
        jp2 = msh->Tri[jTri].Ver[lnofe[jEdge][1]];

        // compare the 4 points
        if(ip1 == jp1 && ip2 == jp2){
          printf("find tri neighbors of %d : %d\n", iTri, jTri);
          msh->Tri[iTri].Voi[iEdge] = jTri;
          break_flag = 1;
          return 1;
        }
      }
    }
  return -1;
}

int msh_better_neighborsQ2(Mesh *msh)
{
  //use more memories to save time
  int iTri,iTet, iVer,i,j,jj, insert_place, insert_index;
  TriVert* ver_index_list; //save the iTri for each vertical
  ver_index_list = (TriVert*)malloc(sizeof(TriVert) * (msh->NbrVer+1));
  memset(ver_index_list, 0, sizeof(TriVert));
  TriVert* temp_TriVert; Triangle* temp_Tri;
  for(iTri = 1; iTri<= msh->NbrTri; iTri ++){
    temp_Tri = msh->Tri + iTri;
    printf("scanning %d of %d\n", iTri,msh->NbrTri);
    for(i = 0; i<3; i++){
      temp_TriVert = ver_index_list + temp_Tri->Ver[i];
      if(temp_TriVert->count < 3)
        temp_TriVert->Tri[temp_TriVert->count] = iTri;
      else{
        if(temp_TriVert)
      }
      temp_TriVert->count ++;
    }

  }
  printf("number of vertical: %d\n", msh->NbrVer);
  int insert_table[3][2] = {{1,2},{0,2},{0,1}};
  for(iVer = 1; iVer<= msh->NbrVer; iVer ++){
    temp_TriVert = ver_index_list + iVer;
    printf("finding neigbhors %d of %d\n", iVer,msh->NbrVer);
    for(i = 0; i<3; i++){
      temp_Tri = msh->Tri + temp_TriVert->Tri[i];
      printf("debug 0\n" );
      for(j = 0; j<2; j++){
        // find is in the voi of temp_tri
        insert_index = temp_TriVert->Tri[insert_table[i][j]];
        printf("voins: %d, %d, %d\n", temp_TriVert->Tri[0],temp_TriVert->Tri[1],temp_TriVert->Tri[2]);
        printf("debug 1, insert_index: %d\n",insert_index);
        insert_place = is_not_in_voi_tri(temp_Tri);
        printf("debug 2\n");
        if( insert_place >=0)
          temp_Tri->Voi[insert_place] = insert_index;
          printf("%d tri is the neighbor of %d tri\n", insert_index ,temp_TriVert->Tri[i]);
      }
    }
  }

}


*/

int  msh_neighborsQ2(Mesh *msh)
{
  int iTet, iFac, jTet, jFac, ip1, ip2 ,ip3 , jp1, jp2, jp3;
  int break_flag = 0;
  int ii,j,temp;
  int is_find_tri;
  int is_find_tet;
  double dstep = (msh->bb[1] - msh->bb[0])/(pow(msh->NbrVer,0.333));
  double x_ave_i, y_ave_i, z_ave_i, x_ave_j, y_ave_j, z_ave_j;
  dstep *= 30;
  if ( ! msh ) return 0;
//int lnofa[4][3] = {{1,2,3},{2,3,0},{3,0,1},{0,1,2}};
  for(iTet=1; iTet<=msh->NbrTet; iTet++) {

    for(iFac=0; iFac<4; iFac++) {
      ip1 = msh->Tet[iTet].Ver[lnofa[iFac][0]];
      ip2 = msh->Tet[iTet].Ver[lnofa[iFac][1]];
      ip3 = msh->Tet[iTet].Ver[lnofa[iFac][2]];

      /* find the Tet different from iTet that has ip1, ip2, ip2 as vertices */
      break_flag = 0;
      //printf("debug 0\n");

      for(jTet=1; jTet<=msh->NbrTet;jTet++) {

        if(msh->Tet[iTet].Ver[0]){
          continue;
        }

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
  int head, another_tet;
  if ( ! msh ) return 0;
  int i_insert, j_insert;
  /* initialize HashTable */
  int debug_porp = 100000;
  int debug_pos = 9084500;
  int debug_switch = 0;
  HashTable* hsh = hash_init(9999991,msh->NbrTet*4);
  int research[4][3] = {{0,1,2},{1,2,3},{2,3,0},{3,0,1}};
  for(iTet=1; iTet<=msh->NbrTet; iTet++) {
    //printf("progress: %d\n",iTet );
    if(iTet%debug_porp ==0)
      printf("progress: %f, current iTet: %d\n",(double)iTet/(double)msh->NbrTet , iTet);
    for(iFac = 0; iFac < 4; iFac++){
    //  printf("iTet: %d, iFac: %d\n", iTet, iFac);
      ip1 = msh->Tet[iTet].Ver[research[iFac][0]];
      ip2 = msh->Tet[iTet].Ver[research[iFac][1]];
      ip3 = msh->Tet[iTet].Ver[research[iFac][2]];
      /* compute the key : ip1+ip2+ip3   */
      /* do we have objects as that key   hash_find () */
      /*  if yes ===> look among objects and potentially update Voi */
      /*  if no  ===> add to hash table.  hash_add()   */
      if(iTet > debug_pos && debug_switch == 1)
        printf("start to find hash\n" );
      head = hash_find(hsh,ip1,ip2,ip3,iTet,iTet > debug_pos && debug_switch == 1); // if exist, add iTet
      if(iTet > debug_pos && debug_switch == 1)
        printf("head: %d\n", head);
      if(head>=0)
      {
        i_insert = 0; j_insert = 0;

        while(i_insert < 4){
        //  printf("i_insert %d\n", i_insert);
          if(msh->Tet[iTet].Voi[i_insert] == 0)
            break;
          i_insert++;
        }
        another_tet = hsh->LstObj[head][3];
        while(j_insert < 4){
        //  printf("j_insert %d\n", j_insert);
          if(msh->Tet[another_tet].Voi[j_insert] == 0)
            break;
          j_insert++;
        }
        msh->Tet[iTet].Voi[i_insert] = another_tet;
        msh->Tet[another_tet].Voi[j_insert] = iTet;
        if(iTet > debug_pos && debug_switch ==1){
          printf("find neighors tet: %d and %d\n",iTet, another_tet );
        }

      }
      else{
        if(iTet >  debug_pos && debug_switch ==1)
          printf("add to hashtable: vert:%d,\t\t%d,\t\t%d,\t\ttet:%d\n",ip1, ip2, ip3, iTet);
        hash_add(hsh, ip1, ip2, ip3, iTet);
        if(iTet > debug_pos && debug_switch == 1)
          printf("finished add\n");
      }

    }
  }
  return 1;
}
