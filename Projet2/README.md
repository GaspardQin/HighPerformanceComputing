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
    source /opt/intel/parallel_studio_xe_2018.2.046/compilers_and_libraries_2018/linux/bin/compilervars.sh intel64

    ```
  - compile icpc
    ```
    icpc -g -std=c++11 -qopt-report=5 -qopenmp -qopt-report-phase=loop,vec,par,openmp -qopt-report-annotate=html main.cpp lectureVilles.cpp prim.cpp -O3
    ```
    -g is for debug

    to use AVX2
    ```
    icpc -g -std=c++11 -qopt-report=5 -qopenmp -qopt-report-phase=loop,vec,par,openmp -qopt-report-annotate=html main.cpp lectureVilles.cpp prim.cpp -O3 -xCORE-AVX2
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
