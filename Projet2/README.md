### What have been modified
- The definition of `resuGraphe.dat`, the attribute `distance` and `inS` is added to each line.

### Features
- Add two new mode of views of result.
  - Show all edges and its weight mode
    To enable this mode, add `#define SHOW_ALL` to `prim.h`
  - Show every step mode
    In this mode, data will be saved in every iteration. All of this data will be saved in a folder `./steps`
    To enable this mode, add `#define SHOW_EVERY_STEPS` to `prim.h`
- Visualize these two new mode by plotlib
  - Show the weight of each edge
  - identify automatically which mode is used to create the data.

### How to use visualization
- Using command:
  - visualize in the `SHOW_EVERY_STEPS` mode
    ```
    python visualisation.py ./steps/ ./resuVilles.dat 0
    ```
    Considering the performance, picture display function is disabled. However, all of the pictures created are saved in the folder `./step_anim`

  - visualize in the other mode
    ```
    python visualisation.py ./resuGraphe.dat ./resuVilles.dat 0
    ```
  - Show weight
    ```
    python visualisation.py ./steps/ ./resuVilles.dat 1
    ```
    or
    ```
    python visualisation.py ./resuGraphe.dat ./resuVilles.dat 1
    ```

### Compile with icpc
  - find icpc
    ```
     source /usr/ensta/bin/intel.sh
    ```

    in my machine
    ```
    source /opt/intel/parallel_studio_xe_2018.2.046/bin/psxevars.sh
    source /opt/intel/vtune_amplifier/amplxe-vars.sh
    ```
  - compile icpc
    ```
    icpc -g -std=c++11 -qopt-report=5 -qopenmp -qopt-report-phase=loop,vec,par,openmp -qopt-report-annotate=html main.cpp lectureVilles.cpp prim.cpp -O3
    ```
    -g is for debug

    to use AVX2
    ```
    icpc -g -std=c++11 -qopt-report=5 -qopenmp -qopt-report-phase=loop,vec,par,openmp -qopt-report-annotate=html main.cpp lectureVilles.cpp prim.cpp -O3 -axCORE-AVX2
    ```

    There exists citis with same lon and lat !!!
### Bottleneck
  - Problem (Version non_amelioration (compiler en o3, minPop = 1000))
    - computeDistance() use 86.9% of total time.
    - iteration in computeDistance() is not vectorized because of accessing `distance[i][j]` and `distance[j][i]` in the same inter-loop.
    - computing `sin` or `cos` consumes 34.1% of total time
    - all of the loop in prim.cpp are not vectorized, because "loop was not vectorized: vectorization possible but seems inefficient."
  - Solution (tag: triangle_matirx)
    - store only a triangle matrix.
    - parallelized outer iteration, vectorized inner iteration of computeDistance

  - Problem
    - No need to compute N^2 times sin and cos.
  - Solution (tag:sin_cos_precomputed)
    - compute and store sin and cos in advance. (vectorized)

  - Problem
    - Memory Bound at 113, 103, 99 lines' loop is too high. (60.7% cpu time)
    - function 'getDistance' influence the pip mechanism (wrong prediction, Not too much, 0.2s in total)
  - Solution (tag: compelete_bloc_matrix_write)
    - need to be aligned access. (it's better to not use triangle matrix)
    - use complete matrix, writing with "bloc operation" (Great speedup)

  - Improvement (tag: use_avx2) v1.4.0
    - not use inS[] to improve aligned access performance.
    - use AVX2 to speed up.

  - Improvement  
    - use the same datatype in the vectorized ligns (change double to float)
    - use `_mm_malloc` , `_mm_free` instead of `new []` and `delete []`, the latter is not standard-aligned.
    [https://software.intel.com/en-us/articles/data-alignment-to-assist-vectorization](https://software.intel.com/en-us/articles/data-alignment-to-assist-vectorization)


  - branch computedistance_realtime
    - do not save the distance matrix, save Memory
    - difficult to parallelize, but enough quick
### Compilation results (n=0, precision: double)
  - `-g -qopenmp` : (in fact, openmp is not used)
    ```
    Total time: 224.372
    distance total: 94273.20546
    ```
  - `icpc -g -std=c++11 main.cpp lectureVilles.cpp prim.cpp`
    ```
    Total time: 227.495
    distance total: 94273.20546
    ```
  - `icpc -O1 -std=c++11 main.cpp lectureVilles.cpp prim.cpp`
    ```
    Total time: 214.476
    distance total: 94273.20546
    ```
  - `icpc -O2 -std=c++11 main.cpp lectureVilles.cpp prim.cpp`
    ```
    Total time: 55.3914
    distance total: 128566.44698
    ```
  - `icpc -O3 -std=c++11 main.cpp lectureVilles.cpp prim.cpp`
    ```
    Total time: 55.2005
    distance total: 128566.44698
    ```
  - `icpc -O3 -std=c++11 main.cpp lectureVilles.cpp prim.cpp -xAVX`
    ```
    Total time: 10.8954
    distance total: 128451.57718
    ```
  - `icpc -O3 -std=c++11 main.cpp lectureVilles.cpp prim.cpp -xcore-avx2`
    ```
    Total time: 9.68159
    distance total: 128566.44698
    ```

  [https://software.intel.com/en-us/forums/intel-fortran-compiler-for-linux-and-mac-os-x/topic/269487](https://software.intel.com/en-us/forums/intel-fortran-compiler-for-linux-and-mac-os-x/topic/269487)
  - `icpc -std=c++11 main.cpp lectureVilles.cpp prim.cpp -O2 -nolib-inline`
    ```
    Total time: 208.766
    distance total: 94273.20546
    ```

    Need to compare log of each Version!!!!
### Discovery
  - I found that `acos(sin*cos ...)` is sensitive to the precision, especially the `acos`, use "double" rather than "float" gives a better precision. In addition, the vectorization operation use the `fma` instruction, who uses a estimation of the value, so it gives a more bad precision, which is not enough for our application.

  - Use `double` solved this problem, but it is double slow. Considering to compare the cost of all using double and the cost of only using double in this formula and the transfom cost from double to float.

  - Result

    - `icpc -g -std=c++11 main.cpp lectureVilles.cpp prim.cpp`
    ```
    Total time: 232.908
    distance total: 91863.15784
    ```
    we can see that using double gives a result slightly different of using float, about 3%.


    - ` icpc -O3 -std=c++11 main.cpp lectureVilles.cpp prim.cpp -xcore-avx2`
    ```
    Total time: 9.70212
    distance total: 91863.15792
    ```
    Almost the same with mode `-g`, the fma use an approx, however thanks to double precision, this approx is safe and only have `8*10^(-10)` difference.
  

### Improvement
