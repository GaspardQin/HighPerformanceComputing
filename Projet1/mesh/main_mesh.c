#include <mesh.h>

int main(int argc, char *argv[])
{
   double to,ti;
   if ( argc < 2 ) {
     printf(" usage : mesh file \n");
     return 0;
   }

   /* read a mesh */
   to =  GetWallClock();
   Mesh * msh = msh_read(argv[1]);
   ti =  GetWallClock();

   if ( ! msh ) return 0;

   printf("  Vertices   %10d \n", msh->NbrVer);
   printf("  Triangles  %10d \n", msh->NbrTri);
   printf("  Tetrahedra %10d \n", msh->NbrTet);
   printf("  time to read the mesh %lg (s) \n",ti-to);

   /* re-order a mesh */
   to =  GetWallClock();
   msh_reorder(msh);
   ti =  GetWallClock();
   printf("  time to re-order the mesh  %lg (s) \n",ti-to);

   /* create neigbhors Q2 version */
   to =  GetWallClock();
   //msh_better_neighborsQ2(msh);
   //msh_neighborsQ2(msh);
   ti =  GetWallClock();
   printf("  time q2 neigh.        %lg (s) \n",ti-to);

   /* create neigbhors with hash table */
   to =  GetWallClock();
   msh_neighbors(msh);
   ti =  GetWallClock();
   printf("  time hash tab neigh.  %lg (s) \n",ti-to);

   /* write reordered mesh */
   to =  GetWallClock();
   msh_write(msh,"output.meshb");
   ti =  GetWallClock();

   return 0;
}
